#include "GltfModelLoader.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE
// STB_IMAGE_IMPLEMENTATION is defined through bx already!
#define TINYGLTF_NO_STB_IMAGE_WRITE 
#include <tinygltf/tiny_gltf.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <bx/file.h>
#include <bimg/bimg.h>
#include <bimg/decode.h>


bgfx::TextureHandle loadTexture(const char* _filePath, uint32_t _flags);

namespace bae {
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
        pRegistry->assign<Materials::TexturedBasic>(entity, processMeshMaterial(folderPath, model, model.meshes[node.mesh]));
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
    // Scaling is not currently supported because **shrug**
    if (node.rotation.size() == 4) {
        // Quaternion
        Rotation rotation = glm::vec4{
                node.rotation[0],
                node.rotation[1],
                node.rotation[2],
                node.rotation[3]
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
        if (indexAccessor.type != TINYGLTF_TYPE_SCALAR || indexAccessor.byteOffset != 0) {
            throw std::runtime_error("Don't know how to handle non uint16_t indices");
        }

        const tinygltf::BufferView& bufferView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
        const bgfx::Memory* indexMemory = bgfx::copy(&buffer.data.at(0) + bufferView.byteOffset, bufferView.byteLength);
        geometry.indexBuffer = bgfx::createIndexBuffer(indexMemory);

    }
    int accessorIndex;    
    
    // Position
    accessorIndex = primitive.attributes.at("POSITION");
    copyBuffer(model ,accessorIndex, geometry, PosVertex::ms_declaration);

    // Normal
    accessorIndex = primitive.attributes.at("NORMAL");
    copyBuffer(model, accessorIndex, geometry, NormalVertex::ms_declaration);

    // Tex Coords
    accessorIndex = primitive.attributes.at("TEXCOORD_0");
    copyBuffer(model, accessorIndex, geometry, TexCoordVertex::ms_declaration);

    return pGeoRegistry->set(mesh.name, geometry);
}

void GltfModelLoader::copyBuffer(
    const tinygltf::Model& model,
    const int accessorIndex,
    Geometry& geometry,
    const bgfx::VertexDecl& decl)
{
    const tinygltf::Accessor& accessor{ model.accessors[accessorIndex] };
    const tinygltf::BufferView& bufferView{ model.bufferViews[accessor.bufferView] };
    const tinygltf::Buffer& buffer{ model.buffers[bufferView.buffer] };

    const bgfx::Memory* vertMemory = bgfx::copy(&(buffer.data.at(bufferView.byteOffset)), bufferView.byteLength);
    geometry.vertexBuffers[geometry.numVertBufferStreams++] = bgfx::createVertexBuffer(vertMemory, decl);
}

Materials::TexturedBasic GltfModelLoader::processMeshMaterial(
    const std::string& folderPath,
    const tinygltf::Model& model,
    const tinygltf::Mesh& mesh)
{
    Materials::TexturedBasic meshMaterial{};
    tinygltf::Material material = model.materials[mesh.primitives[0].material];
    for (const auto& matValue : material.values) {
        if (matValue.first.compare("baseColorTexture") == 0) {
            size_t index = static_cast<size_t>(matValue.second.json_double_value.at("index"));
            const tinygltf::Texture& texture = model.textures[index];
            // TODO: Handle sampler options
            //if (model.samplers.size > 0) {
            //    const tinygltf::Sampler& sampler = model.samplers[texture.sampler];
            //}
            const tinygltf::Image& image = model.images[texture.source];
            const std::string fileName = folderPath + image.uri;

            bgfx::TextureHandle handle = loadTexture(fileName.c_str(), BGFX_SAMPLER_NONE);
            pTextureManager->addTexture(handle);

            meshMaterial.setBaseColor(handle);
        }
    }
    return meshMaterial;
}
}

void* load(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const char* _filePath, uint32_t* _size)
{
    if (bx::open(_reader, _filePath))
    {
        uint32_t size = (uint32_t)bx::getSize(_reader);
        void* data = BX_ALLOC(_allocator, size);
        bx::read(_reader, data, size);
        bx::close(_reader);
        if (NULL != _size)
        {
            *_size = size;
        }
        return data;
    }
    else
    {
        std::string err = "Failed to open: ";
        err.append(_filePath);
        throw std::runtime_error(err);
    }

    if (NULL != _size)
    {
        *_size = 0;
    }

    return NULL;
}

bgfx::TextureHandle loadTexture(const char* _filePath, uint32_t _flags)
{
    bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
    auto _reader = bae::FileReader{};
    auto allocator = bx::DefaultAllocator{};

    uint32_t size;
    void* data = load(&_reader, &allocator, _filePath, &size);
    if (nullptr != data)
    {
        
        bimg::ImageContainer* imageContainer = bimg::imageParse(&allocator, data, size);
        //bool parsed = bimg::imageParse(imageContainer, data, size);

        if (nullptr != &imageContainer)
        {
            const bgfx::Memory* mem = bgfx::copy(
                imageContainer->m_data,
                imageContainer->m_size
            );
            BX_FREE(&allocator, data);

            if (imageContainer->m_cubeMap)
            {
                handle = bgfx::createTextureCube(
                    uint16_t(imageContainer->m_width),
                    1 < imageContainer->m_numMips,
                    imageContainer->m_numLayers,
                    bgfx::TextureFormat::Enum(imageContainer->m_format),
                    _flags,
                    mem
                );
            }
            else if (1 < imageContainer->m_depth)
            {
                handle = bgfx::createTexture3D(
                    uint16_t(imageContainer->m_width)
                    , uint16_t(imageContainer->m_height)
                    , uint16_t(imageContainer->m_depth)
                    , 1 < imageContainer->m_numMips
                    , bgfx::TextureFormat::Enum(imageContainer->m_format)
                    , _flags
                    , mem
                );
            }
            else if (bgfx::isTextureValid(0, false, imageContainer->m_numLayers, bgfx::TextureFormat::Enum(imageContainer->m_format), _flags))
            {
                handle = bgfx::createTexture2D(
                    uint16_t(imageContainer->m_width)
                    , uint16_t(imageContainer->m_height)
                    , 1 < imageContainer->m_numMips
                    , imageContainer->m_numLayers
                    , bgfx::TextureFormat::Enum(imageContainer->m_format)
                    , _flags
                    , mem
                );
            }

            if (bgfx::isValid(handle))
            {
                bgfx::setName(handle, _filePath);
            }
        }
    }

    return handle;
}
