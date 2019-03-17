#pragma once
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace bae
{
struct PointLightEmitter {
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 10.0f;
};

struct LightUniformSet {
    std::string lightName;
    uint8_t lightCount = 0;
    uint16_t maxLightCount = 10;  // Has to match whatever we have set in the shader...

    // params.x = lightCount
    bgfx::UniformHandle params = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle lightPos = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle lightColorIntensity = BGFX_INVALID_HANDLE;

    std::vector<glm::vec4> lightPosData;
    std::vector<glm::vec4> lightColorIntensityData;

    void init();
    inline void destroy()
    {
        bgfx::destroy(params);
        bgfx::destroy(lightPos);
        bgfx::destroy(lightColorIntensity);
    }

    void set();
};
}  // namespace bae
