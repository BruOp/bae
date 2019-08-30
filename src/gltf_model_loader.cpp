#include "gltf_model_loader.h"

#include <stdexcept>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "bgfx_utils.h"
#include "PhysicallyBasedScene.h"

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

namespace bae
{
    struct GltfToBgfxAttributeMaps
    {
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

    enum struct TransparencyMode
    {
        OPAQUE_,
        MASKED,
        BLENDED,
    };

    typedef std::vector<std::pair<PBRMaterial, TransparencyMode>> MaterialsList;

    Model loadGltfModel(const std::string& assetPath, const std::string& fileName)
    {
        Model output_model{};

        tinygltf::TinyGLTF loader;
        std::string err, warn;
        tinygltf::Model gltf_model;
        bool res = loader.LoadASCIIFromFile(&gltf_model, &err, &warn, fileName);
        TransparencyMode transparency_mode = TransparencyMode::OPAQUE_;

        if (!warn.empty()) {
            std::cout << warn << std::endl;
        }

        if (!err.empty()) {
            std::cout << err << std::endl;
        }

        if (!res) {
            throw std::runtime_error("Failed to load GLTF Model");
        }

        const tinygltf::Scene& scene = gltf_model.scenes[gltf_model.defaultScene];

        // Load in dummy files to use for materials that do not have texture present
        // Allows us to treat all our materials the same way
        std::string dummyFiles[] = {
            "textures/dummy_white.dds",
            "textures/dummy_metallicRoughness.dds",
            "textures/dummy_normal_map.dds",
        };
        const size_t DUMMY_TEXTURE_COUNT = BX_COUNTOF(dummyFiles);

        // The plus 3 is due to our dummy textures
        output_model.textures.reserve(gltf_model.textures.size() + DUMMY_TEXTURE_COUNT);
        for (const auto& fileName : dummyFiles) {
            bgfx::TextureHandle handle = loadTexture(fileName.c_str());
            output_model.textures.push_back(handle);
        }
        // TEXTURES
        for (const tinygltf::Texture& texture : gltf_model.textures) {
            std::string& uri = gltf_model.images[texture.source].uri;

            // Ignore the sampling options for filter -- always use mag: LINEAR and min: LINEAR_MIPMAP_LINEAR
            uint64_t flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_ANISOTROPIC;

            /*
            For future generations: if you're familiar with the OpenGL/WebGL sampler options, the way that
            BGFX structures this mapping is to map [SAMPLER_MIN, has_mips + SAMPLER_MIP] to a 2D matrix of
            all the different possibilities. So if you want LINEAR_MIPMAP_LINEAR, this is equivalent to
            BGFX_SAMPLER_MIN_ANISOTROPIC. Meanwhile, if you want NEAREST_MIPMAP_NEAREST, you'd use
            BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MIP_POINT. Whenever you want LINEAR, you don't need a
            flag UNLESS it's to access the LINEAR_MIPMAP_* flags...
            */

            if (texture.sampler != -1) {
                const tinygltf::Sampler& sampler = gltf_model.samplers[texture.sampler];

                switch (sampler.wrapS) {
                case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                    flags |= BGFX_SAMPLER_U_CLAMP;
                case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
                    flags |= BGFX_SAMPLER_U_MIRROR;
                default:
                    // Default is repeat
                    break;
                }
                switch (sampler.wrapT) {
                case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                    flags |= BGFX_SAMPLER_V_CLAMP;
                case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
                    flags |= BGFX_SAMPLER_V_MIRROR;
                default:
                    // Default is repeat
                    break;
                }
            }

            bgfx::TextureInfo textureInfo;
            bgfx::TextureHandle handle = loadTexture(uri.c_str(), flags, 0u, &textureInfo);
            output_model.textures.push_back(handle);
        }

        MaterialsList materials_list;
        materials_list.reserve(gltf_model.materials.size());

        // MATERIALS
        for (const tinygltf::Material& material : gltf_model.materials) {
            // NOTE: We do not respect texCoord values other than the default 0... sorry!
            // Set default values
            PBRMaterial materialData{
                { 1.0f, 1.0f, 1.0f, 1.0f }, // baseColorFactor
                { 0.0f, 0.0f, 0.0f, 1.0f }, // emissiveFactor 
                0.5f, // alphaCutoff
                1.0f, // metallicFactor
                1.0f, // roughnessFactor
                output_model.textures[0], // baseColorTexture as dummy_white
                output_model.textures[1], // metallicRoughnessTexture as dummy_metallicRoughness;
                output_model.textures[2], // normalTexture as dummy_normal_map;
                output_model.textures[0], // emissiveTexture as dummy_white;
                output_model.textures[0], // occlusionTexture as dummy_white;
            };

            auto valuesEnd = material.values.end();
            auto p_keyValue = material.values.find("baseColorTexture");
            if (p_keyValue != valuesEnd) {
                materialData.baseColorTexture = output_model.textures[p_keyValue->second.TextureIndex() + DUMMY_TEXTURE_COUNT];
            };

            p_keyValue = material.values.find("baseColorFactor");
            if (p_keyValue != valuesEnd) {
                auto& data = p_keyValue->second.ColorFactor();
                materialData.baseColorFactor = glm::vec4{
                    static_cast<float>(data[0]),
                    static_cast<float>(data[1]),
                    static_cast<float>(data[2]),
                    static_cast<float>(data[3]),
                };
            }

            p_keyValue = material.values.find("metallicRoughnessTexture");
            if (p_keyValue != valuesEnd) {
                materialData.metallicRoughnessTexture = output_model.textures[p_keyValue->second.TextureIndex() + DUMMY_TEXTURE_COUNT];
            }

            p_keyValue = material.values.find("metallicFactor");
            if (p_keyValue != valuesEnd) {
                materialData.metallicFactor = static_cast<float>(p_keyValue->second.Factor());
            }

            // Additional Factors

            valuesEnd = material.additionalValues.end();
            p_keyValue = material.additionalValues.find("normalTexture");
            if (p_keyValue != valuesEnd) {
                materialData.normalTexture = output_model.textures[p_keyValue->second.TextureIndex() + DUMMY_TEXTURE_COUNT];
            }

            auto p_keyValue = material.additionalValues.find("emissiveTexture");
            if (p_keyValue != valuesEnd) {
                materialData.emissiveTexture = output_model.textures[p_keyValue->second.TextureIndex() + DUMMY_TEXTURE_COUNT];

                if (material.additionalValues.find("emissiveFactor") != valuesEnd) {
                    auto& data = material.additionalValues.at("emissiveFactor").ColorFactor();
                    materialData.emissiveFactor = glm::vec4(
                        static_cast<float>(data[0]),
                        static_cast<float>(data[1]),
                        static_cast<float>(data[2]),
                        1.0f
                    );
                }
            };

            p_keyValue = material.additionalValues.find("occlusionTexture");
            if (p_keyValue != valuesEnd) {
                materialData.baseColorFactor = glm::make_vec4(p_keyValue->second.ColorFactor().data());
            }

            p_keyValue = material.additionalValues.find("metallicRoughnessTexture");
            if (p_keyValue != valuesEnd) {
                materialData.metallicRoughnessTexture = output_model.textures[p_keyValue->second.TextureIndex() + DUMMY_TEXTURE_COUNT];
            }

            p_keyValue = material.additionalValues.find("alphaMode");
            if (p_keyValue != valuesEnd) {
                if (p_keyValue->second.string_value == "BLEND") {
                    transparency_mode = TransparencyMode::BLENDED;
                } else if (p_keyValue->second.string_value == "MASK") {
                    transparency_mode = TransparencyMode::MASKED;
                }
            }

            p_keyValue = material.additionalValues.find("alphaCutoff");
            if (p_keyValue != valuesEnd) {
                materialData.alphaCutoff = static_cast<float>(p_keyValue->second.Factor());
            }

            materials_list.emplace_back(materialData, transparency_mode);
        }

        // For each node in the scene
        for (const int node_idx : scene.nodes) {
            loadModelNode(output_model, gltf_model, gltf_model.nodes[node_idx], glm::identity<glm::mat4>(), materials_list);
        }

        //loadMeshes(assetPath, aScene);

    }

    void loadModelNode(Model& output_model, const tinygltf::Model& gltf_model, const tinygltf::Node& node, glm::mat4 parentTransform, const MaterialsList& materials_list)
    {
        // Process the transform
        glm::mat4 transform = processTransform(node, parentTransform);

        // Process the mesh (resulting in a Mesh, Material, RenderState and Transform?)
        if (node.mesh != -1) {
            const tinygltf::Mesh& mesh = gltf_model.meshes[node.mesh];

            for (const tinygltf::Primitive& primitive : mesh.primitives) {
                if (primitive.material != -1) {
                    Mesh newMesh = processPrimitive(gltf_model, primitive);

                    const std::pair<PBRMaterial, TransparencyMode>& materialModePair = materials_list[primitive.material];

                    if (materialModePair.second == TransparencyMode::BLENDED) {
                        output_model.transparentMeshes.meshes.push_back(newMesh);
                        output_model.transparentMeshes.materials.push_back(materialModePair.first);
                        output_model.transparentMeshes.transforms.push_back(transform);
                    } else if (materialModePair.second == TransparencyMode::MASKED) {
                        output_model.maskedMeshes.meshes.push_back(newMesh);
                        output_model.maskedMeshes.materials.push_back(materialModePair.first);
                        output_model.maskedMeshes.transforms.push_back(transform);
                    } else {
                        output_model.opaqueMeshes.meshes.push_back(newMesh);
                        output_model.opaqueMeshes.materials.push_back(materialModePair.first);
                        output_model.opaqueMeshes.transforms.push_back(transform);
                    }
                }
            }
        }

        for (int child_idx : node.children) {
            // Process the children (using the Transform) recursively
            loadModelNode(output_model, gltf_model, gltf_model.nodes[child_idx], transform, materials_list);
        }
    }


    // Given a GLTF primitive, return a mesh
    // TODO: Targets and weights
    Mesh processPrimitive(const tinygltf::Model& gltf_model, const tinygltf::Primitive& primitive)
    {
        Mesh mesh{};

        // Get indices
        {
            const tinygltf::Accessor& indexAccessor = gltf_model.accessors[primitive.indices];
            if (indexAccessor.type != TINYGLTF_TYPE_SCALAR || indexAccessor.componentType != TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT) {
                throw std::runtime_error("Don't know how to handle non uint16_t indices");
            }

            const tinygltf::BufferView& bufferView = gltf_model.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer& buffer = gltf_model.buffers[bufferView.buffer];
            const bgfx::Memory* indexMemory = bgfx::copy(&buffer.data.at(0) + bufferView.byteOffset, bufferView.byteLength);
            mesh.indexHandle = bgfx::createIndexBuffer(indexMemory);
        }

        const std::string ATTRIBUTE_NAMES[] = {
            "POSITION",
            "NORMAL",
            "TANGENT",
            "TEXCOORD_0"
        };

        for (size_t i; i < BX_COUNTOF(ATTRIBUTE_NAMES); ++i) {
            const std::string& attrName = ATTRIBUTE_NAMES[i];

            int accessorIndex = primitive.attributes.at(attrName);
            const tinygltf::Accessor& accessor{ gltf_model.accessors[accessorIndex] };

            // Copy Associated Memory
            const tinygltf::BufferView& bufferView{ gltf_model.bufferViews[accessor.bufferView] };
            const tinygltf::Buffer& buffer{ gltf_model.buffers[bufferView.buffer] };

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
            mesh.vertexHandles[i] = bgfx::createVertexBuffer(vertMemory, decl);
        }

        return mesh;
    }

    // Returns a transformation matrix for a given GLTF node
    // Order of operations (right to left) in glTF: parentTransform * (T * R * S)
    glm::mat4 processTransform(const tinygltf::Node& node, const glm::mat4& parentTransform)
    {
        glm::mat4 localTransform = glm::identity<glm::mat4>();
        if (node.scale.size() == 3) {
            localTransform = glm::scale(localTransform, glm::vec3{ node.scale[0], node.scale[0], node.scale[0] });
        }

        if (node.rotation.size() == 4) {
            // Quaternion
            glm::quat rotation{
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

        return parentTransform * localTransform;
    }

    //tinygltf::LoadImageDataFunction loadImageData(
    //	tinygltf::Image* image,
    //	const int image_idx,
    //	std::string* err,
    //	std::string* warn,
    //	int req_width,
    //	int req_height,
    //	const unsigned char* bytes,
    //	int size,
    //	void* user_data
    //) {
    //	bgfx::TextureInfo textureInfo{};

    //	bgfx::
    //}
}