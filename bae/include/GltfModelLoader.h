#pragma once
#include <entt/entt.hpp>
#include <memory>

#include "Geometry.h"
#include "Materials.h"
#include "Texture.h"
#include "utils/Common.h"

namespace bgfx
{
class VertexDecl;
}

namespace tinygltf
{
class Node;
class Model;
class Mesh;
}  // namespace tinygltf

namespace bae
{
class GltfModelLoader
{
   public:
    GltfModelLoader(entt::DefaultRegistry &registry, GeometryRegistry &geoRegistry, TextureManager &textureManager);

    std::vector<Entity> loadModel(const std::string &folderPath, const std::string &modelName);

   private:
    entt::DefaultRegistry *pRegistry = nullptr;
    GeometryRegistry *pGeoRegistry = nullptr;
    TextureManager *pTextureManager = nullptr;

    tinygltf::Model loadFile(const std::string &filePath);

    void processModelNodes(
        const std::string &folderPath,
        std::vector<Entity> &entities,
        const tinygltf::Model &model,
        const tinygltf::Node &node,
        const Entity parent,
        const Transform &parentTransform);

    Transform processTransform(
        const Entity entity,
        const tinygltf::Node &node,
        const Entity parent,
        const Transform &parentTransform);

    Geometry processMeshGeometry(const tinygltf::Model &model, const tinygltf::Mesh &mesh);

    void copyBuffer(
        const tinygltf::Model &model,
        const int accessorIndex,
        Geometry &geometry,
        const bgfx::VertexDecl &decl);

    Materials::TexturedPhysical processMeshMaterial(
        const std::string &folderPath,
        const tinygltf::Model &model,
        const tinygltf::Mesh &mesh);
};

class GltfTextureLoader
{
   public:
    bgfx::TextureHandle loadTextureByIndex(const size_t index) const;

    const tinygltf::Model *pModel;
    const std::string folderPath;
};
}  // namespace bae
