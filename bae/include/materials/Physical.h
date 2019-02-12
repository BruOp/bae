#pragma once
#include <algorithm>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

#include "MaterialType.h"
#include "utils/Common.h"

namespace bae {
namespace Materials {
    extern MaterialType physical;

    struct Physical {

        const static MaterialType* materialType;
        const static UniformInfoMap uniformInfoMap;

        Physical(
            const glm::vec4& color = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
            const float metallic = 0.0,
            const float roughness = 0.045,
            const float reflectance = 0.5)
            : matColor{ colorToLinear(color) }
            , metallic{ metallic }
            , roughness{ std::clamp(roughness, minRoughness, maxRoughness) }
            , reflectance{ reflectance }
            , u_matColor{ materialType->getHandle("matColor") }
            , u_metallicRoughnessReflectance{ materialType->getHandle("metallicRoughnessReflectance") }
        {
        }

        inline void setUniforms() const
        {
            float metallicRoughnessReflectance[] = { metallic, roughness, reflectance, 0.0 };
            bgfx::setUniform(u_matColor, &(matColor[0]));
            bgfx::setUniform(u_metallicRoughnessReflectance, metallicRoughnessReflectance);
        };

        inline float getMetallic() { return metallic; }
        inline float getRoughness() { return roughness; }
        inline float getReflectance() { return reflectance; }

        inline void setRoughness(const float roughness)
        {
            metallic = std::clamp(roughness, minRoughness, maxRoughness);
        }
        inline void setMatColor(const glm::vec4& color)
        {
            matColor = colorToLinear(color);
        }
        inline void setMetallic(const float newMetallic) { metallic = newMetallic; }
        inline void setReflectance(const float newReflectance) { reflectance = reflectance; }

    private:
        glm::vec4 matColor;
        float metallic;
        float roughness;
        float reflectance;

        bgfx::UniformHandle u_matColor = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle u_metallicRoughnessReflectance = BGFX_INVALID_HANDLE;

        static constexpr float minRoughness = 0.045; // To prevent divide by zero errors
        static constexpr float maxRoughness = 1.0;
    };
}
}
