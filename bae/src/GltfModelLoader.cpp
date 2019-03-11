#include "GltfModelLoader.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE
// STB_IMAGE_IMPLEMENTATION is defined through bx already!
#define TINYGLTF_NO_STB_IMAGE_WRITE 
#include <tinygltf/tiny_gltf.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "utils/FileUtils.h"

namespace bae {

struct GltfToBgfxAttributeMaps {    
    std::unordered_map<std::string, bgfx::Attrib::Enum> attributes;
    std::unordered_map<int, uint8_t> sizes;
    std::unordered_map<int, bgfx::AttribType::Enum> componentType;
};

const GltfToBgfxAttributeMaps GLTF_TO_BGFX_ATTRIBUTE_MAPS{
    {
        {"POSITION", bgfx::Attrib::Position},
        {"NORMAL", bgfx::Attrib::Normal},
        {"TANGENT", bgfx::Attrib::Tangent},
        {"TEXCOORD_0", bgfx::Attrib::TexCoord0},
    },
    {
        { TINYGLTF_TYPE_SCALAR, 1 },
        { TINYGLTF_TYPE_VEC2, 2 },
        { TINYGLTF_TYPE_VEC3, 3 },
        { TINYGLTF_TYPE_VEC4, 4 },
    },
    {
        {TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE, bgfx::AttribType::Uint8},
        {TINYGLTF_PARAMETER_TYPE_FLOAT, bgfx::AttribType::Float},
        // Cannot handle TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT or any of the others
    },
};

GltfModelLoader::GltfModelLoader(entt::DefaultRegistry& registry, GeometryRegistry& geoRegistry, TextureManager& textureManager)
    : pRegistry{ &registry }
    , pGeoRegistry{ &geoRegistry }
    , pTextureManager{ &textureManager }

{
}

std::vector<Entity> GltfModelLoader::loadModel(const std::string& folderPath, const std::string& modelName)
{
    tinygltf::Model model = loadFile(folderPath + modelName + ".gltf");
    std::vector<uint32_t> entities{};
    
    entities.reserve(model.nodes.size());

    // Root nodes only
    const tinygltf::Scene& scene = model.scenes[model.defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        // Recursive traversal from each root
        processModelNodes(
            folderPath,
            entities,
            model,
            model.nodes[scene.nodes[i]],
            entt::null,
            glm::identity<glm::mat4>()
        );
    }
    return entities;

}

tinygltf::Model GltfModelLoader::loadFile(const std::string& filePath)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader{};
    std::string err;
    std::string warn;

    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filePath.c_str());
    if (!warn.empty()) {
        throw std::runtime_error("WARN: " + warn);
    }
    if (!err.empty()) {
        throw std::runtime_error("ERR: " + err);
    }
    if (!res) {
        throw std::runtime_error("Failed to load glTF: " + filePath);
    }
    return model;
}

void GltfModelLoader::processModelNodes(
    const std::string& folderPath, 
    std::vector<Entity>& entities,
    const tinygltf::Model& model,
    const tinygltf::Node& node,
    const Entity parent,
    const Transform& parentTransform)
{
    auto entity = pRegistry->create();
    entities.push_back(entity);

    Transform globalTransform = processTransform(entity, node, parent, parentTransform);
    
    if (node.mesh >= 0) {
        pRegistry->assign<Geometry>(entity, processMeshGeometry(model, model.meshes[node.mesh]));
        pRegistry->assign<Materials::TexturedPhysical>(entity, processMeshMaterial(folderPath, model, model.meshes[node.mesh]));
    }

    for (size_t i = 0; i < node.children.size(); ++i) {
        const tinygltf::Node& childNode = model.nodes[node.children[i]];
        processModelNodes(folderPath, entities, model, childNode, entity, globalTransform);
    }
}

Transform GltfModelLoader::processTransform(
    const Entity entity,
    const tinygltf::Node& node,
    const Entity parent,
    const Transform& parentTransform)
{
    Transform localTransform = glm::identity<glm::mat4>();
    // Order of operations (right to left) in glTF: T * R * S
    if (node.scale.size() == 3) {
        localTransform = glm::scale(
            localTransform,
            glm::vec3{
                node.scale[0],
                node.scale[1],
                node.scale[2],
            }
        );
    }

    if (node.rotation.size() == 4) {
        // Quaternion
        Rotation rotation{
                static_cast<float>(node.rotation[3]),
                static_cast<float>(node.rotation[0]),
                static_cast<float>(node.rotation[1]),
                static_cast<float>(node.rotation[2]),
        };
        localTransform = glm::toMat4(rotation) * localTransform;
    }

    if (node.translation.size() == 3) {
        localTransform = glm::translate(
            localTransform,
            glm::vec3{
                node.translation[0],
                node.translation[1],
                node.translation[2]
            });
    }

    Transform globalTransform;
    if (parent != entt::null) {
        globalTransform = parentTransform * localTransform;
        pRegistry->assign<LinkedTransform>(entity, localTransform, parent);
    }
    else {
        globalTransform = localTransform;
    }
    return pRegistry->assign<Transform>(entity, globalTransform);
}

Geometry GltfModelLoader::processMeshGeometry(const tinygltf::Model& model, const tinygltf::Mesh& mesh)
{
    Geometry geometry{};
    // Get indices
    if (mesh.primitives.size() > 1) {
        throw std::runtime_error("Don't know how to handle meshes with more than one primitive set");
    }
    const tinygltf::Primitive& primitive = mesh.primitives[0];

    // Get indices
    {
        const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
        if (indexAccessor.type != TINYGLTF_TYPE_SCALAR || indexAccessor.componentType != TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT) {
            throw std::runtime_error("Don't know how to handle non uint16_t indices");
        }

        const tinygltf::BufferView& bufferView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
        const bgfx::Memory* indexMemory = bgfx::copy(&buffer.data.at(0) + bufferView.byteOffset, bufferView.byteLength);
        geometry.indexBuffer = bgfx::createIndexBuffer(indexMemory);

    }

    const std::string ATTRIBUTE_NAMES[] = {
        "POSITION",
        "NORMAL",
        "TANGENT",
        "TEXCOORD_0"
    };

    for (auto& attrName : ATTRIBUTE_NAMES) {
        int accessorIndex = primitive.attributes.at(attrName);
        const tinygltf::Accessor& accessor{ model.accessors[accessorIndex] };
        
        // Copy Associated Memory
        const tinygltf::BufferView& bufferView{ model.bufferViews[accessor.bufferView] };
        const tinygltf::Buffer& buffer{ model.buffers[bufferView.buffer] };
        if (geometry.numVertBufferStreams >= bae::MAX_VERT_BUFFERS_PER_GEOMETRY) {
            throw std::runtime_error("Don't have any more vertex buffers available for this Geometry!");
        }
        const bgfx::Memory* vertMemory = bgfx::copy(&(buffer.data.at(bufferView.byteOffset)), bufferView.byteLength);

        // Define vertex specification/declaration for bgfx
        bgfx::VertexDecl decl;
        decl.begin()
            .add(
                GLTF_TO_BGFX_ATTRIBUTE_MAPS.attributes.at(attrName),
                GLTF_TO_BGFX_ATTRIBUTE_MAPS.sizes.at(accessor.type),
                GLTF_TO_BGFX_ATTRIBUTE_MAPS.componentType.at(accessor.componentType),
                accessor.normalized
            )
            .end();

        // Create the buffer and store the handles in our geometry object
        geometry.vertexBuffers[geometry.numVertBufferStreams++] = bgfx::createVertexBuffer(vertMemory, decl);
    }
    // Store a copy in our geometry registry to ensure cleanup.
    return pGeoRegistry->set(mesh.name, geometry);
}

Materials::TexturedPhysical GltfModelLoader::processMeshMaterial(
    const std::string& folderPath,
    const tinygltf::Model& model,
    const tinygltf::Mesh& mesh)
{
    GltfTextureLoader textureLoader{ &model, folderPath };
    Materials::TexturedPhysical meshMaterial{};
    tinygltf::Material material = model.materials[mesh.primitives[0].material];
    for (const auto& [name, values] : material.values) {
        size_t index = static_cast<size_t>(values.json_double_value.at("index"));
        bgfx::TextureHandle handle = textureLoader.loadTextureByIndex(index);
        pTextureManager->addTexture(handle);
        
        if (name.compare("baseColorTexture") == 0) {
            meshMaterial.setBaseColor(handle);
        }
        else if (name.compare("metallicRoughnessTexture") == 0) {
            meshMaterial.setOccRoughMetal(handle);
        }
    }

    for (const auto& [name, values] : material.additionalValues) {
        if (name.compare("normalTexture") == 0) {
            size_t index = static_cast<size_t>(values.json_double_value.at("index"));
            bgfx::TextureHandle handle = textureLoader.loadTextureByIndex(index);
            pTextureManager->addTexture(handle);

            meshMaterial.setNormalMap(handle);
        }
    }
    return meshMaterial;
}

bgfx::TextureHandle GltfTextureLoader::loadTextureByIndex(const size_t index) const{
    const tinygltf::Texture& texture = pModel->textures[index];

    const tinygltf::Image& image = pModel->images[texture.source];
    const std::string fileName = folderPath + (image.uri.substr(0, image.uri.size() - 4).append(".dds"));

    bgfx::TextureHandle handle = loadTexture(fileName.c_str(), BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE);
    return handle;
}
}
