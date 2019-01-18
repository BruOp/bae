#pragma once
#include "Light.h"
#include "utils/Geometric.h"
#include <entt/entt.hpp>

namespace bae {
struct LightUniformSet {
    std::string lightName;
    uint8_t lightCount = 0;
    uint16_t maxLightCount = 10; // Has to match whatever we have set in the shader...

    // params.x = lightCount
    bgfx::UniformHandle params = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle lightPos = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle lightColorIntensity = BGFX_INVALID_HANDLE;

    std::vector<glm::vec4> lightPosData;
    std::vector<glm::vec4> lightColorIntensityData;

    void init()
    {
        auto uniformName = lightName + "_params";
        params = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4);
        uniformName = lightName + "_lightPos";
        lightPos = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4, maxLightCount);
        uniformName = lightName + "_lightColorIntensity";
        lightColorIntensity = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4, maxLightCount);

        lightPosData.resize(maxLightCount);
        lightColorIntensityData.resize(maxLightCount);
    }

    inline void destroy()
    {
        bgfx::destroy(params);
        bgfx::destroy(lightPos);
        bgfx::destroy(lightColorIntensity);
    }

    void set()
    {
        float paramsArr[4]{
            float(lightCount),
            0.0f,
            0.0f,
            0.0f
        };
        bgfx::setUniform(params, paramsArr);
        bgfx::setUniform(lightPos, lightPosData.data(), maxLightCount);
        bgfx::setUniform(lightColorIntensity, lightColorIntensityData.data(), maxLightCount);
    }
};
}
