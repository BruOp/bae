/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <bx/rng.h>
#include "bgfx_utils.h"
#include "common.h"
#include "imgui/imgui.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <exception>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "camera.h"


namespace example
{
#define SAMPLER_POINT_CLAMP (BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP)

	static float s_texelHalf = 0.0f;

	struct Vertex {
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec4 tangent;
		glm::vec4 bitangent;

		static void init()
		{
			ms_decl.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true)
				.add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Float, true)
				.add(bgfx::Attrib::Bitangent, 4, bgfx::AttribType::Float, true)
				.end();
		}

		static bgfx::VertexDecl ms_decl;
	};

	bgfx::VertexDecl Vertex::ms_decl;

	struct UniformHandleInfo {
		bgfx::UniformType::Enum type;
		bgfx::UniformHandle handle = BGFX_INVALID_HANDLE;
	};

	typedef std::unordered_map<std::string, UniformHandleInfo> UniformInfoMap;

	void init(UniformInfoMap& uniformInfo) {
		for (auto& entry : uniformInfo) {
			const std::string& name = entry.first;
			UniformHandleInfo& info = entry.second;
			info.handle = bgfx::createUniform(name.c_str(), info.type);
		}
	}

	void destroy(UniformInfoMap& uniformInfo) {
		for (auto& entry : uniformInfo) {
			bgfx::UniformHandle handle = entry.second.handle;
			if (bgfx::isValid(handle)) {
				bgfx::destroy(handle);
			}
		}
	}

	struct MaterialType {
		std::string name;
		bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

		UniformInfoMap uniformInfo;
	};

	void init(MaterialType& materialType) {
		std::string vs_name = "vs_" + materialType.name;
		std::string fs_name = "fs_" + materialType.name;
		materialType.program = loadProgram(vs_name.c_str(), fs_name.c_str());

		init(materialType.uniformInfo);
	}

	void destroy(MaterialType& materialType) {
		destroy(materialType.uniformInfo);

		if (bgfx::isValid(materialType.program)) {
			bgfx::destroy(materialType.program);
		}

	}

	bgfx::UniformHandle getUniformHandle(const MaterialType& materialType, const std::string& uniformName)
	{
		return materialType.uniformInfo.at(uniformName).handle;
	}

	struct Material
	{
		std::string name;
		bgfx::TextureHandle diffuse = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle specular = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle normal = BGFX_INVALID_HANDLE;
		bool hasAlpha = false;

		static MaterialType matType;
	};

	MaterialType Material::matType{
		"phong",
		BGFX_INVALID_HANDLE,
		{
			{"diffuseMap", {bgfx::UniformType::Sampler}},
			{"specularMap", {bgfx::UniformType::Sampler}},
			{"normalMap", {bgfx::UniformType::Sampler}},
		},
	};

	void setUniforms(const Material& material) {
		bgfx::setTexture(0, getUniformHandle(material.matType, "diffuseMap"), material.diffuse);
		bgfx::setTexture(1, getUniformHandle(material.matType, "normalMap"), material.normal);
		bgfx::setTexture(2, getUniformHandle(material.matType, "specularMap"), material.specular);
	}

	struct Mesh
	{
		bgfx::VertexBufferHandle vertexHandle = BGFX_INVALID_HANDLE;
		bgfx::IndexBufferHandle indexHandle = BGFX_INVALID_HANDLE;

		Material material;
	};

	void destroy(Mesh& mesh) {
		bgfx::destroy(mesh.vertexHandle);
		bgfx::destroy(mesh.indexHandle);
	}

	template<class Resource>
	class ResourceList
	{
	public:
		void add(const std::string& name, Resource resource) {
			resources[name] = resource;
		}

		Resource get(const std::string& name) {
			return resources[name];
		}

		bool has(const std::string& name) const {
			return resources.find(name) != resources.end();
		};

		void reserve(size_t newSize) {
			resources.reserve(newSize);
		}

		void destroy() {
			for (auto& pair : resources) {
				bgfx::destroy(pair.second);
			}
		}

	private:
		std::unordered_map<std::string, Resource> resources;
	};

	class Scene {
	public:
		ResourceList<bgfx::TextureHandle> textures;

		std::vector<Material> materials;
		std::vector<Mesh> meshes;


		void loadMaterials(const std::string& assetPath, const aiScene* aScene) {
			// Load in dummy files to use for materials that do not have texture present
			// Allows us to treat all our materials the same way
			std::vector<std::string> dummyFiles{
				"sponza/dummy.dds",
				"sponza/dummy_specular.dds",
				"sponza/dummy_ddn.dds",
			};
			std::vector<bgfx::TextureHandle> dummyHandles(3);

			for (size_t i = 0; i < dummyFiles.size(); i++) {
				const auto& fileName = dummyFiles[i];
				bgfx::TextureHandle handle = loadTexture((assetPath + fileName).c_str(), BGFX_SAMPLER_UVW_CLAMP);
				textures.add(fileName, handle);
				dummyHandles[i] = handle;
			}

			const bgfx::TextureHandle& dummyDiffuse = dummyHandles[0];
			const bgfx::TextureHandle& dummySpecular = dummyHandles[1];
			const bgfx::TextureHandle& dummyNormal = dummyHandles[2];

			materials.reserve(aScene->mNumMaterials);
			for (uint32_t i = 0; i < aScene->mNumMaterials; i++) {
				aiMaterial* aMaterial = aScene->mMaterials[i];
				aiString name;
				aMaterial->Get(AI_MATKEY_NAME, name);

				Material material{
					name.C_Str()
				};

				aiString textureFile;
				aMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &textureFile);
				if (aScene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
					std::string fileName = std::string(textureFile.C_Str());
					std::replace(fileName.begin(), fileName.end(), '\\', '/');

					if (!textures.has(fileName)) {
						std::string filePath = assetPath + fileName;
						bgfx::TextureHandle handle = loadTexture(filePath.c_str(), BGFX_SAMPLER_UVW_CLAMP);
						textures.add(fileName, handle);
						material.diffuse = handle;
					} else {
						material.diffuse = textures.get(fileName);
					}
				}
				else
				{
					std::cout << "  Material has no diffuse, using dummy texture!" << std::endl;
					material.diffuse = dummyDiffuse;
				}

				aMaterial->GetTexture(aiTextureType_SPECULAR, 0, &textureFile);
				if (aScene->mMaterials[i]->GetTextureCount(aiTextureType_SPECULAR) > 0) {
					std::string fileName = std::string(textureFile.C_Str());
					std::replace(fileName.begin(), fileName.end(), '\\', '/');

					if (!textures.has(fileName)) {
						std::string filePath = assetPath + fileName;
						bgfx::TextureHandle handle = loadTexture(filePath.c_str(), BGFX_SAMPLER_UVW_CLAMP);
						textures.add(fileName, handle);
						material.specular = handle;
					}
					else {
						material.specular = textures.get(fileName);
					}
				}
				else
				{
					std::cout << "  Material has no specular, using dummy texture!" << std::endl;
					material.diffuse = dummySpecular;
				}

				aMaterial->GetTexture(aiTextureType_NORMALS, 0, &textureFile);
				if (aScene->mMaterials[i]->GetTextureCount(aiTextureType_NORMALS) > 0) {
					std::string fileName = std::string(textureFile.C_Str());
					std::replace(fileName.begin(), fileName.end(), '\\', '/');

					if (!textures.has(fileName)) {
						std::string filePath = assetPath + fileName;
						bgfx::TextureHandle handle = loadTexture(filePath.c_str(), BGFX_SAMPLER_UVW_CLAMP);
						textures.add(fileName, handle);
						material.normal = handle;
					}
					else {
						material.normal = textures.get(fileName);
					}
				}
				else
				{
					std::cout << "  Material has no normal map, using dummy texture!" << std::endl;
					material.normal = dummyNormal;
				}

				if (aMaterial->GetTextureCount(aiTextureType_OPACITY) > 0) {
					material.hasAlpha = true;
				}

				materials.push_back(material);
			}


		};

		void loadMeshes(const aiScene* aScene)
		{
			meshes.reserve(aScene->mNumMeshes);
			for (size_t i = 0; i < aScene->mNumMeshes; i++) {
				aiMesh* aMesh = aScene->mMeshes[i];

				bool hasUV = aMesh->HasTextureCoords(0);
				bool hasTangent = aMesh->HasTangentsAndBitangents();

				std::vector<Vertex> vertices(aMesh->mNumVertices);
				//vertices.reserve(aMesh->mNumVertices);
				for (size_t j = 0; j < aMesh->mNumVertices; j++) {

					glm::vec2 texCoords{0.0f};
					if (hasUV) {
						texCoords.x = aMesh->mTextureCoords[0][j].x;
						texCoords.y = aMesh->mTextureCoords[0][j].y;
					}

					glm::vec4 tangent = glm::vec4(0.0f);
					glm::vec4 bitangent = glm::vec4(0.0f);
					if (hasTangent) {
						tangent = {
							aMesh->mTangents[j].x, aMesh->mTangents[j].y, aMesh->mTangents[j].z, 0.0f
						};
						bitangent = {
							aMesh->mBitangents[j].x, aMesh->mBitangents[j].y, aMesh->mBitangents[j].z, 0.0f
						};
					}
					auto& vert = aMesh->mVertices[j];
					vertices[j] = Vertex{
						glm::vec3{vert.x, vert.y, vert.z},
						texCoords,
						glm::make_vec3(&(aMesh->mNormals[j].x)),
						tangent,
						bitangent,
					};
				}

				std::vector<uint16_t> indices(size_t(aMesh->mNumFaces) * 3);
				for (size_t j = 0; j < aMesh->mNumFaces; j++) {
					indices[j * 3 + 0] = uint16_t(aMesh->mFaces[j].mIndices[0]);
					indices[j * 3 + 1] = uint16_t(aMesh->mFaces[j].mIndices[1]);
					indices[j * 3 + 2] = uint16_t(aMesh->mFaces[j].mIndices[2]);
				}

				const bgfx::Memory* vertMemory = bgfx::copy(vertices.data(), uint32_t(vertices.size()) * sizeof(Vertex));
				const bgfx::Memory* indexMemory = bgfx::copy(indices.data(), uint32_t(indices.size()) * sizeof(uint16_t));

				Mesh mesh{
					bgfx::createVertexBuffer(vertMemory, Vertex::ms_decl),
					bgfx::createIndexBuffer(indexMemory),
					materials[aMesh->mMaterialIndex],
				};

				meshes.push_back(mesh);
			}
		};


	};

	void destroy(Scene& scene) {
		for (auto& mesh : scene.meshes) {
			destroy(mesh);
		}

		scene.textures.destroy();
	}

	Scene loadScene() {
		Assimp::Importer importer;
		int flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded;
		std::string assetPath = "meshes/";
		const aiScene* aScene = importer.ReadFile(assetPath + "sponza.dae", flags);

		BX_CHECK(aScene == nullptr, "Could not open sponza file");

		Scene scene;
		scene.loadMaterials(assetPath, aScene);
		scene.loadMeshes(aScene);
		return scene;
	}
	class ExampleForward : public entry::AppI
	{
	public:
		ExampleForward(const char* _name, const char* _description) : entry::AppI(_name, _description) {}

		void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
		{
			Args args(_argc, _argv);

			m_width = _width;
			m_height = _height;
			m_debug = BGFX_DEBUG_TEXT;
			m_reset = BGFX_RESET_VSYNC;

			bgfx::Init initInfo;
			initInfo.type = args.m_type;
			initInfo.vendorId = args.m_pciId;
			initInfo.resolution.width = m_width;
			initInfo.resolution.height = m_height;
			initInfo.resolution.reset = m_reset;
			bgfx::init(initInfo);

			// Enable m_debug text.
			bgfx::setDebug(m_debug);

			// Set view 0 clear state.
			bgfx::setViewClear(0
				, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
				, 0x303030ff
				, 1.0f
				, 0
			);

			const bgfx::Caps* caps = bgfx::getCaps();
			m_computeSupported = !!(caps->supported & BGFX_CAPS_COMPUTE);

			if (!m_computeSupported) {
				return;
			}

			// Create vertex stream declaration.
			Vertex::init();

			example::init(Material::matType);

			// Lets load all the meshes
			m_scene = loadScene();

			// Imgui.
			imguiCreate();

			m_caps = bgfx::getCaps();
			s_texelHalf = bgfx::RendererType::Direct3D9 == m_caps->rendererType ? 0.5f : 0.0f;

			// Init camera
			cameraCreate();
			cameraSetPosition({ 0.0f, 1.5f, -200.0f });

			m_oldWidth = 0;
			m_oldHeight = 0;
			m_oldReset = m_reset;

			m_time = 0.0f;
		}

		virtual int shutdown() override
		{
			// Cleanup.
			imguiDestroy();
			destroy(m_scene);
			destroy(Material::matType);

			cameraDestroy();

			if (!m_computeSupported) {
				return 0;
			}

			// Shutdown bgfx.
			bgfx::shutdown();

			return 0;
		}

		bool update() override
		{
			if (entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState)) {
				return false;
			}

			if (!m_computeSupported) {
				return false;
			}

			if (m_oldWidth != m_width || m_oldHeight != m_height || m_oldReset != m_reset) {
				// Recreate variable size render targets when resolution changes.
				m_oldWidth = m_width;
				m_oldHeight = m_height;
				m_oldReset = m_reset;
			}

			imguiBeginFrame(m_mouseState.m_mx
				,  m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				,  m_mouseState.m_mz
				, uint16_t(m_width)
				, uint16_t(m_height)
				);

			showExampleDialog(this);

			imguiEndFrame();

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);

			int64_t now = bx::getHPCounter();
			static int64_t last = now;
			const int64_t frameTime = now - last;
			last = now;
			const double freq = double(bx::getHPFrequency());
			const float deltaTime = (float)(frameTime / freq);
			m_time += deltaTime;

			float proj[16];
			bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 1000.0f, bgfx::getCaps()->homogeneousDepth);

			// Update camera
			float view[16];

			cameraUpdate(deltaTime, m_mouseState);
			cameraGetViewMtx(view);
			// Set view and projection matrix
			bgfx::setViewTransform(0, view, proj);


			glm::mat4 mtx = glm::identity<glm::mat4>();
			//bx::mtxRotateXY(glm::value_ptr(mtx), 0.0f, 0.2f * m_time);

			uint64_t state = 0
				| BGFX_STATE_WRITE_RGB
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_LESS
				//| BGFX_STATE_PT_POINTS
				| BGFX_STATE_CULL_CCW;
				//| BGFX_STATE_MSAA;

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

			// Only using one material type for now, otherwise we'd have to navigate through the meshes' material
			bgfx::ProgramHandle program = Material::matType.program;

			for (auto& mesh : m_scene.meshes) {
				bgfx::setTransform(glm::value_ptr(mtx));
				bgfx::setIndexBuffer(mesh.indexHandle);
				bgfx::setVertexBuffer(0, mesh.vertexHandle);
				setUniforms(mesh.material);
				bgfx::setState(state);
				bgfx::submit(0, program);
			}

			bx::Vec3 cameraPosition = cameraGetPosition();

			bgfx::frame();

			return true;
		}

		entry::MouseState m_mouseState;

		bx::RngMwc m_rng;

		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_debug;
		uint32_t m_reset;

		uint32_t m_oldWidth;
		uint32_t m_oldHeight;
		uint32_t m_oldReset;

		Scene m_scene;

		bool m_computeSupported = true;

		const bgfx::Caps* m_caps;
		float m_time;
	};

}  // namespace example

ENTRY_IMPLEMENT_MAIN(
	example::ExampleForward,
	"02-forward-rendering",
	"Rendering sponza using single lighting pass forward.");
