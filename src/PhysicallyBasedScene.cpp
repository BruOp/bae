#include "PhysicallyBasedScene.h"
#include <stdexcept>
#include <iostream>

#include "bgfx_utils.h"

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

namespace bae {
    void Vertex::init()
    {
        ms_decl.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true)
            .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float, true)
            .add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float, true)
            .end();
    }

    bgfx::VertexDecl Vertex::ms_decl;

  //  bgfx::TextureHandle loadAssimpTexture(
  //      const aiMaterial* aMaterial,
  //      const aiTextureType textureType,
		//const unsigned int index,
  //      const std::string& assetPath,
  //      ResourceList<bgfx::TextureHandle>& textures
  //  ) {
  //      aiString textureFile;
  //      aiTextureMapMode mapModes[3];
		//
  //      if (aMaterial->GetTextureCount(textureType) <= 0) {
  //          return BGFX_INVALID_HANDLE;
  //      }

  //      aMaterial->GetTexture(textureType, 0, &textureFile, NULL, NULL, NULL, NULL, mapModes);

  //      std::string fileName = std::string(textureFile.C_Str());
  //      std::replace(fileName.begin(), fileName.end(), '\\', '/');

  //      if (!textures.has(fileName)) {
  //          std::string filePath = assetPath + fileName;
  //          uint64_t flags = 0 | BGFX_SAMPLER_MAG_ANISOTROPIC;
  //          if (textureType == aiTextureType_DIFFUSE) {
  //              flags |= BGFX_TEXTURE_SRGB;
  //          }

  //          if (mapModes[0] == aiTextureMapMode_Mirror) {
  //              flags |= BGFX_SAMPLER_U_MIRROR;
  //          }
  //          else if (mapModes[0] == aiTextureMapMode_Clamp) {
  //              flags |= BGFX_SAMPLER_U_CLAMP;
  //          }

  //          if (mapModes[1] == aiTextureMapMode_Mirror) {
  //              flags |= BGFX_SAMPLER_V_MIRROR;
  //          }
  //          else if (mapModes[1] == aiTextureMapMode_Clamp) {
  //              flags |= BGFX_SAMPLER_V_CLAMP;
  //          }

  //          bgfx::TextureHandle handle = loadTexture(filePath.c_str(), flags);
  //          if (!bgfx::isValid(handle)) {
  //              throw std::runtime_error("Texture could not be loaded!: " + fileName);
  //          }

  //          textures.add(fileName, handle);
  //          return handle;
  //      }
  //      else {
  //          return textures.get(fileName);
  //      }
  //  }

	

  //  PBRMaterial Scene::loadMaterial(const std::string& assetPath, const aiMaterial* aMaterial)
  //  {

  //      aiString name;
  //      aMaterial->Get(AI_MATKEY_NAME, name);

  //      PBRMaterial material{};

  //      bgfx::TextureHandle handle = loadAssimpTexture(aMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, assetPath, textures);
  //      if (bgfx::isValid(handle)) {
  //          material.baseColorTexture = handle;
  //      }
  //      else {
  //          std::cout << "  Material has no diffuse, using dummy texture!" << std::endl;
  //          material.baseColorTexture = textures.get("textures/dummy.dds");
  //      }

  //      handle = loadAssimpTexture(aMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, assetPath, textures);
  //      if (bgfx::isValid(handle)) {
  //          material.metallicRoughnessTexture = handle;
  //      }
  //      else {
  //          std::cout << "  Material has no Metallic Roughness, using dummy texture!" << std::endl;
  //          material.metallicRoughnessTexture = textures.get("textures/dummy_metallicRoughness.dds");
  //      }

  //      handle = loadAssimpTexture(aMaterial, aiTextureType_NORMALS, 0, assetPath, textures);
  //      if (bgfx::isValid(handle)) {
  //          material.normalTexture = handle;
  //      }
  //      else {
  //          std::cout << "  Material has no normal map, using dummy texture!" << std::endl;
  //          material.normalTexture = textures.get("textures/dummy_ddn.dds");
  //      }

		//handle = loadAssimpTexture(aMaterial, AI_MATKEY_GLTF_EM)

  //      return material;
  //  };

  //  void Scene::loadMeshes(const std::string& assetPath, const aiScene* aScene)
  //  {
  //      for (size_t i = 0; i < aScene->mNumMeshes; i++) {
  //          aiMesh* aMesh = aScene->mMeshes[i];

  //          bool hasUV = aMesh->HasTextureCoords(0);
  //          bool hasTangent = aMesh->HasTangentsAndBitangents();

  //          std::vector<Vertex> vertices(aMesh->mNumVertices);
  //          for (size_t j = 0; j < aMesh->mNumVertices; j++) {

  //              glm::vec2 texCoords{ 0.0f };
  //              if (hasUV) {
  //                  texCoords.x = aMesh->mTextureCoords[0][j].x;
  //                  texCoords.y = aMesh->mTextureCoords[0][j].y;
  //              }

  //              glm::vec4 tangent = glm::vec4(0.0f);
  //              glm::vec4 bitangent = glm::vec4(0.0f);
  //              if (hasTangent) {
  //                  tangent = {
  //                      aMesh->mTangents[j].x, aMesh->mTangents[j].y, aMesh->mTangents[j].z, 0.0f
  //                  };
  //                  bitangent = {
  //                      aMesh->mBitangents[j].x, aMesh->mBitangents[j].y, aMesh->mBitangents[j].z, 0.0f
  //                  };
  //              }

  //              vertices[j] = Vertex{
  //                  glm::make_vec3(&aMesh->mVertices[j].x),
  //                  texCoords,
  //                  glm::make_vec3(&aMesh->mNormals[j].x),
  //                  glm::make_vec3(&aMesh->mTangents[j].x),
  //                  glm::make_vec3(&aMesh->mBitangents[j].x)
  //              };
  //          }

  //          std::vector<uint16_t> indices(size_t(aMesh->mNumFaces) * 3);
  //          for (size_t j = 0; j < aMesh->mNumFaces; j++) {
  //              indices[j * 3 + 0] = uint16_t(aMesh->mFaces[j].mIndices[0]);
  //              indices[j * 3 + 1] = uint16_t(aMesh->mFaces[j].mIndices[1]);
  //              indices[j * 3 + 2] = uint16_t(aMesh->mFaces[j].mIndices[2]);
  //          }

  //          const bgfx::Memory* vertMemory = bgfx::copy(vertices.data(), uint32_t(vertices.size()) * sizeof(Vertex));
  //          const bgfx::Memory* indexMemory = bgfx::copy(indices.data(), uint32_t(indices.size()) * sizeof(uint16_t));

  //          Mesh mesh{
  //              bgfx::createVertexBuffer(vertMemory, Vertex::ms_decl),
  //              bgfx::createIndexBuffer(indexMemory),
  //          };

  //          const aiMaterial* aMaterial = aScene->mMaterials[aMesh->mMaterialIndex];

  //          PBRMaterial material = loadMaterial(assetPath, aMaterial);

  //          aiString aiAlphaMode;
  //          if (aMaterial->Get(AI_MATKEY_GLTF_ALPHAMODE, aiAlphaMode) == AI_SUCCESS) {
  //              std::string alphaMode = aiAlphaMode.C_Str();
  //              if (alphaMode.compare("MASK") == 0 || alphaMode.compare("BLEND") == 0) {
  //                  transparentMeshes.meshes.push_back(mesh);
  //                  transparentMeshes.materials.push_back(material);
  //              }
  //              else {
  //                  opaqueMeshes.meshes.push_back(mesh);
  //                  opaqueMeshes.materials.push_back(material);
  //              }
  //          }
  //      }
  //  };

	
	void destroy(const Mesh& mesh)
    {
        bgfx::destroy(mesh.vertexHandle);
        bgfx::destroy(mesh.indexHandle);
    }

    void destroy(Scene& scene)
    {
        for (const Mesh& mesh : scene.opaqueMeshes.meshes) {
            destroy(mesh);
        }
        for (const Mesh& mesh : scene.transparentMeshes.meshes) {
            destroy(mesh);
        }

        scene.textures.destroy();
    }
}