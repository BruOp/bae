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


        inline bgfx::UniformHandle getUniformHandle(const std::string& uniformName)
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
        bool hasAlpha = false;

        static MaterialType matType;
    };

    void setUniforms(const Material& material);

    struct Mesh
    {
        bgfx::VertexBufferHandle vertexHandle = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle indexHandle = BGFX_INVALID_HANDLE;

        Material material;
    };

    void destroy(Mesh& mesh);

    class Scene {
    public:
        ResourceList<bgfx::TextureHandle> textures;

        std::vector<Material> materials;
        std::vector<Mesh> meshes;

        void load(const std::string& assetPath, const std::string& fileName);
    private:
        void loadMaterials(const std::string& assetPath, const aiScene* aScene);
        void loadMeshes(const aiScene* aScene);
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

    class LightSet {
    public:
        uint8_t lightCount = 0;
        uint16_t maxLightCount = 255;  // Has to match whatever we have set in the shader...

        // params is bacasically just used to store params.x = lightCount
        bgfx::UniformHandle params = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle lightPos = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle lightColorIntensity = BGFX_INVALID_HANDLE;

        void init(const std::string& lightName)
        {
            auto uniformName = lightName + "_params";
            params = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4);
            uniformName = lightName + "_pos";
            lightPos = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4, maxLightCount);
            uniformName = lightName + "_colorIntensity";
            lightColorIntensity = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4, maxLightCount);

            lightPosData.resize(maxLightCount);
            lightColorIntensityData.resize(maxLightCount);
        }

        void setUniforms() const
        {
            float paramsArr[4]{ float(lightCount), 0.0f, 0.0f, 0.0f };
            bgfx::setUniform(params, paramsArr);
            bgfx::setUniform(lightPos, lightPosData.data(), maxLightCount);
            bgfx::setUniform(lightColorIntensity, lightColorIntensityData.data(), maxLightCount);
        };

        void destroy()
        {
            bgfx::destroy(params);
            bgfx::destroy(lightPos);
            bgfx::destroy(lightColorIntensity);
        }

        bool addLight(const glm::vec3& color, const float intensity, const glm::vec3& position) {
            if (lightCount >= maxLightCount) {
                return false;
            }

            lightPosData[lightCount] = glm::vec4{ position, 1.0 };
            lightColorIntensityData[lightCount] = glm::vec4{ color, intensity };
            ++lightCount;
            return true;
        }

        std::vector<glm::vec4> lightPosData;
        std::vector<glm::vec4> lightColorIntensityData;
    };
}
