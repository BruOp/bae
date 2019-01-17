#pragma once
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <exception>

#include "Geometry.h"
#include "utils/Vertex.h"

namespace bae {

class ModelLoader {
public:
    ModelLoader() = default;
    ModelLoader(GeometryRegistry* geoRegistry)
        : geometryRegistry{ geoRegistry } {};

    Geometry loadModel(const std::string& name, const std::string& file)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(file,
            aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_GenNormals | aiProcess_FlipUVs);

        // If the import failed, report it
        if (!scene) {
            throw std::runtime_error(importer.GetErrorString());
        }

        return processMesh(name, scene);
    }

    Geometry processMesh(const std::string& name, const aiScene* scene)
    {
        std::vector<PosTexNormalVertex> vertices;
        std::vector<uint16_t> indices;

        aiMesh* mesh = scene->mMeshes[0];

        vertices.resize(mesh->mNumVertices);

        for (size_t i = 0; i < mesh->mNumVertices; i++) {
            // process vertex positions, normals and texture coordinates
            vertices[i].pos = glm::vec3{
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z,
            };

            aiVector3D normal = mesh->mNormals[i];
            vertices[i].normal = glm::vec3{ normal.x, normal.y, normal.z };
        }

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            for (size_t i = 0; i < mesh->mNumVertices; i++) {
                vertices[i].u = mesh->mTextureCoords[0][i].x;
                vertices[i].v = mesh->mTextureCoords[0][i].y;
            }
        }

        // process indices
        for (size_t i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        geometryRegistry->create(name, vertices, indices);
        return geometryRegistry->get(name);
    }

private:
    GeometryRegistry* geometryRegistry = nullptr;
};
}
