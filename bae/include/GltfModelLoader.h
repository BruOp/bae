#pragma once
#include <memory>
#include <entt/entt.hpp>
#include <bx/file.h>
#include <bx/filepath.h>

#include "Utils/Common.h"
#include "Geometry.h"
#include "Texture.h"
#include "Materials.h"


namespace bgfx {
    class VertexDecl;
}

namespace tinygltf {
    class Node;
    class Model;
    class Mesh;
}

namespace bae {

    class GltfModelLoader {
    public:
        GltfModelLoader(entt::DefaultRegistry& registry, GeometryRegistry& geoRegistry, TextureManager& textureManager);

        std::vector<Entity> loadModel(const std::string& folderPath, const std::string& modelName);

    private:
        entt::DefaultRegistry* pRegistry = nullptr;
        GeometryRegistry* pGeoRegistry = nullptr;
        TextureManager* pTextureManager = nullptr;
        
        tinygltf::Model loadFile(const std::string& filePath);

        void processModelNodes(
            const std::string& folderPath,
            std::vector<Entity>& entities,
            const tinygltf::Model& model,
            const tinygltf::Node& node,
            const Entity parent,
            const Transform& parentTransform
        );

        Transform processTransform(
            const Entity entity,
            const tinygltf::Node& node,
            const Entity parent,
            const Transform& parentTransform);

        Geometry processMeshGeometry(const tinygltf::Model& model, const tinygltf::Mesh& mesh);

        void copyBuffer(const tinygltf::Model& model, const int accessorIndex, Geometry& geometry, const bgfx::VertexDecl& decl);
        Materials::TexturedBasic processMeshMaterial(const std::string& folderPath, const tinygltf::Model& model, const tinygltf::Mesh& mesh);
    };


    class FileReader : public bx::FileReader
    {
        typedef bx::FileReader super;

    public:
        virtual bool open(const bx::FilePath& _filePath, bx::Error* _err) override
        {
            return super::open(_filePath.get(), _err);
        }
    };

}