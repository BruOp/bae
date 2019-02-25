#pragma once
#include <memory>
#include <entt/entt.hpp>

#include "Utils/Common.h"
#include "Geometry.h"

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
        GltfModelLoader(entt::DefaultRegistry& registry, GeometryRegistry& geoRegistry);

        std::vector<Entity> loadModel(const std::string& filePath);

    private:
        entt::DefaultRegistry* pRegistry = nullptr;
        GeometryRegistry* pGeoRegistry = nullptr;
        
        tinygltf::Model loadFile(const std::string& filePath);

        void processModelNodes(
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
        //void processMeshMaterial(const tinygltf::Model& model, const tinygltf::Mesh& mesh);
    };
}