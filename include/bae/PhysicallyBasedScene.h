#pragma once
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <stdexcept>

#include "ResourceList.h"

namespace bae
{
    struct Mesh
    {
        // Different handle for each "stream" of vertex attributes
        // 0 - Position
        // 1 - Normal
        // 2 - Tangent
        // 3 - TexCoord0
        bgfx::VertexBufferHandle vertexHandles[4] = {
            BGFX_INVALID_HANDLE,
            BGFX_INVALID_HANDLE,
            BGFX_INVALID_HANDLE,
            BGFX_INVALID_HANDLE,
        };
        bgfx::IndexBufferHandle indexHandle = BGFX_INVALID_HANDLE;
        uint8_t numVertexHandles = 0;

        static const uint8_t maxVertexHandles = 4;

        void addVertexHandle(const bgfx::VertexBufferHandle vbh)
        {
            if (numVertexHandles < maxVertexHandles) {
                vertexHandles[numVertexHandles++] = vbh;
            } else {
                throw std::runtime_error("Cannot add additional vertex handle to this mesh.");
            }
        }

        void setBuffers() const
        {
            bgfx::setIndexBuffer(indexHandle);
            for (uint8_t j = 0; j < numVertexHandles; ++j) {
                bgfx::setVertexBuffer(j, vertexHandles[j]);
            }
        }
    };

    void destroy(const Mesh& mesh);

    // Struct containing material information according to the GLTF spec
    // Note: Doesn't fully support the spec :)
    struct PBRMaterial
    {
        // Uniform Data
        glm::vec4 baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec4 emissiveFactor = { 0.0f, 0.0f, 0.0f, 1.0f };
        float alphaCutoff = 1.0f;
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        bgfx::TextureHandle baseColorTexture = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle metallicRoughnessTexture = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle normalTexture = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle emissiveTexture = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle occlusionTexture = BGFX_INVALID_HANDLE;
    };

    struct MeshGroup
    {
        std::vector<PBRMaterial> materials;
        std::vector<Mesh> meshes;
        std::vector<glm::mat4> transforms;
    };
    
    struct Model
    {
        std::vector<bgfx::TextureHandle> textures;

        MeshGroup opaqueMeshes;
        MeshGroup maskedMeshes;
        MeshGroup transparentMeshes;
    };

    void destroy(Model& model);
}
