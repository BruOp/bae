#pragma once
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "ResourceList.h"

namespace bae
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;

        static void init();

        static bgfx::VertexDecl ms_decl;
    };

    class Program
    {
    public:
        virtual void init() = 0;
        virtual void destroy() = 0;

    private:
        ~Program() = default;
    };

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
    };

    void destroy(Mesh& mesh);

    template<typename Material>
    struct MeshGroup
    {
        std::vector<Material> materials;
        std::vector<Mesh> meshes;
        std::vector<glm::mat4> transforms;

        void destroy()
        {
            for (const Mesh& mesh : meshes) {
                destroy(mesh);
            }
        }
    };

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

    struct Model
    {
        std::vector<bgfx::TextureHandle> textures;

        MeshGroup<PBRMaterial> opaqueMeshes;
        MeshGroup<PBRMaterial> maskedMeshes;
        MeshGroup<PBRMaterial> transparentMeshes;
    };

    void destroy(Model& model);
}
