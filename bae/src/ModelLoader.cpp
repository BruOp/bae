#include "ModelLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
// STB_IMAGE_IMPLEMENTATION is defined through bx already!
#define TINYGLTF_NO_STB_IMAGE_WRITE 
#include <tinygltf/tiny_gltf.h>

namespace bae {
	Geometry bae::ModelLoader::loadObjModel(const std::string& name, const std::string& file) noexcept
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str(), nullptr, true)) {
			throw std::runtime_error(err);
		}

		std::vector<PosTexNormalVertex> vertices{};
		std::vector<uint16_t> indices{};

		std::unordered_map<PosTexNormalVertex, uint32_t> uniqueVertices = {};

		auto& shape = shapes[0];
		for (const auto& index : shape.mesh.indices) {

			PosTexNormalVertex vertex{
				{ attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2] },
				0,
				0,
				{ attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2] },
			};
			if (index.texcoord_index >= 0) {
				vertex.u = attrib.texcoords[2 * index.texcoord_index];
				vertex.v = attrib.texcoords[2 * index.texcoord_index + 1];
			}

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}

		pGeometryRegistry->create(name, vertices, indices);
		return pGeometryRegistry->get(name);
	}

	std::vector<Geometry> ModelLoader::loadGltfModel(const std::string& file) noexcept
	{
		std::vector<Geometry> geometries{};
		tinygltf::TinyGLTF loader{};
		tinygltf::Model model;
		std::string err;
		std::string warn;

		bool res = loader.LoadASCIIFromFile(&model, &err, &warn, file.c_str());
		if (!warn.empty()) {
			throw std::runtime_error("WARN: " + warn);
		}
		if (!err.empty()) {
			throw std::runtime_error("ERR: " + err);
		}
		if (!res) {
			throw std::runtime_error("Failed to load glTF: " + file);
		}

		const tinygltf::Scene& scene = model.scenes[model.defaultScene];
		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			processModelNodes(geometries, model, model.nodes[scene.nodes[i]]);
		}
		return geometries;
	}

	void ModelLoader::processModelNodes(std::vector<Geometry>& geometries, const tinygltf::Model& model, const tinygltf::Node& node) noexcept
	{
		if (node.mesh >= 0) {
			processMeshGeometry(geometries, model, model.meshes[node.mesh]);
	    }
		if (node.children.size() > 0) {
			// Can process children nodes recursively
		}
	}


	void ModelLoader::processMeshGeometry(std::vector<Geometry>& geometries, const tinygltf::Model& model, const tinygltf::Mesh& mesh) noexcept
	{
		Geometry geometry{};
		// Get indices
		if (mesh.primitives.size() > 1) {
			throw std::runtime_error("Don't know how to handle meshes with more than one primitive set");
		}
		tinygltf::Primitive primitive = mesh.primitives[0];

		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];
		if (indexAccessor.type != TINYGLTF_TYPE_SCALAR || indexAccessor.byteOffset != 0) {
			throw std::runtime_error("Don't know how to handle non uint16_t indices");
		}
		
		tinygltf::BufferView bufferView = model.bufferViews[indexAccessor.bufferView];
		tinygltf::Buffer buffer = model.buffers[bufferView.buffer];

		const bgfx::Memory* indexMemory = bgfx::copy(&buffer.data.at(0) + bufferView.byteOffset, bufferView.byteLength);
		geometry.indexBuffer = bgfx::createIndexBuffer(indexMemory);

		// Position
		int accessorIndex = primitive.attributes.at("POSITION");
		tinygltf::Accessor accessor = model.accessors[accessorIndex];
		bufferView = model.bufferViews[accessor.bufferView];

		const bgfx::Memory* vertMemory = bgfx::copy(&(buffer.data.at(bufferView.byteOffset)), bufferView.byteLength);
		geometry.vertexBuffers[geometry.numVertBufferStreams++] = bgfx::createVertexBuffer(vertMemory, PosVertex::ms_declaration);

        // Normal
        accessorIndex = primitive.attributes.at("NORMAL");
        accessor = model.accessors[accessorIndex];
        bufferView = model.bufferViews[accessor.bufferView];

        const bgfx::Memory* normalMemory = bgfx::copy(&(buffer.data.at(bufferView.byteOffset)), bufferView.byteLength);
        geometry.vertexBuffers[geometry.numVertBufferStreams++] = bgfx::createVertexBuffer(normalMemory, NormalVertex::ms_declaration);

        
        // Tex Coords
        accessorIndex = primitive.attributes.at("TEXCOORD_0");
        accessor = model.accessors[accessorIndex];
        bufferView = model.bufferViews[accessor.bufferView];

        const bgfx::Memory* texCoordMemory = bgfx::copy(&(buffer.data.at(bufferView.byteOffset)), bufferView.byteLength);
        geometry.vertexBuffers[geometry.numVertBufferStreams++] = bgfx::createVertexBuffer(texCoordMemory, TexCoordVertex::ms_declaration);
		   
        processMeshMaterial(model, mesh);

        pGeometryRegistry->set(mesh.name, geometry);
		geometries.push_back(geometry);
	}

    void ModelLoader::processMeshMaterial(const tinygltf::Model& model, const tinygltf::Mesh& mesh) noexcept {
        tinygltf::Material material = model.materials[mesh.primitives[0].material];
        for (const auto& mats : material.values) {
            if (mats.first.compare("baseColorTexture")) {
                //tinygltf::Texture texture = model.textures[mats.second.json_double_value];
            }
        }        
    }
}