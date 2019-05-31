#pragma once
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

#include "ResourceList.h"

namespace bae {

    struct Vertex {
        glm::vec3 position;
        glm::vec2 uv;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;

        static void init();

        static bgfx::VertexDecl ms_decl;
    };

    struct UniformHandleInfo {
        bgfx::UniformType::Enum type;
        bgfx::UniformHandle handle = BGFX_INVALID_HANDLE;
    };

    typedef std::unordered_map<std::string, UniformHandleInfo> UniformInfoMap;

    void init(UniformInfoMap& uniformInfo);
    void destroy(UniformInfoMap& uniformInfo);


    struct MaterialType {
        std::string name;
        bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

        UniformInfoMap uniformInfo;


        inline bgfx::UniformHandle getUniformHandle(const std::string& uniformName) const
        {
            return uniformInfo.at(uniformName).handle;
        }
    };

    void init(MaterialType& matType);
    void destroy(MaterialType& matType);

    struct Material
    {
        std::string name;
        bgfx::TextureHandle diffuse = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle metallicRoughness = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle normal = BGFX_INVALID_HANDLE;
    };

    void setUniforms(const Material& material, const MaterialType& matType);

    struct Mesh
    {
        bgfx::VertexBufferHandle vertexHandle = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle indexHandle = BGFX_INVALID_HANDLE;
    };

    void destroy(Mesh& mesh);

    template<class Material>
    struct MeshGroup {
        std::vector<Material> materials;
        std::vector<Mesh> meshes;
    };

    class Scene {
    public:
        ResourceList<bgfx::TextureHandle> textures;

        MeshGroup<Material> opaqueMeshes;
        MeshGroup<Material> transparentMeshes;

        // These aren't considered "owned" by the scene, so the won't get destroyed
        // when this scene is.
        MaterialType opaqueMatType;
        MaterialType transparentMatType;

        void load(const std::string& assetPath, const std::string& fileName);
    private:
        Material loadMaterial(const std::string& assetPath, const aiMaterial* aMaterial);
        void loadMeshes(const std::string& assetPath, const aiScene* aScene);
    };

    void destroy(Scene& scene);

    struct SceneUniforms {
        bgfx::UniformHandle cameraPos = BGFX_INVALID_HANDLE;
    };

    inline void init(SceneUniforms& sceneUniforms) {
        sceneUniforms.cameraPos = bgfx::createUniform("cameraPos", bgfx::UniformType::Vec4);
    };

    inline void destroy(SceneUniforms& sceneUniforms) {
        bgfx::destroy(sceneUniforms.cameraPos);
    }

}
