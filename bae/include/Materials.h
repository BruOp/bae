#pragma once
#include <algorithm>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

#include "MaterialType.h"
#include "Uniforms.h"
#include "utils/Common.h"

namespace bae {
namespace Materials {
    class BaseMaterial {
        inline virtual void setUniforms() const = 0;
    };

    struct Basic : public BaseMaterial {
        Vec4Uniform color;

        static MaterialType materialType;

        Basic(const glm::vec4& color = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f })
            : color{ color, materialType.getHandle("matColor") }
        {
        }

        inline void setUniforms() const override
        {
            color.setUniform();
        };
    };

    struct Lambertian : public BaseMaterial {
        friend class MaterialTypeManager;

        Vec4Uniform color;

        static MaterialType materialType;

        Lambertian(const glm::vec4& color = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f })
            : color{ color, materialType.getHandle("matColor") }
        {
        }

        inline void setUniforms() const override
        {
            color.setUniform();
        };

        inline static bgfx::ProgramHandle getProgram()
        {
            return materialType.program;
        }
    };

    class Physical : public BaseMaterial {
    public:
        Physical(
            const glm::vec4& color = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
            const float metallic = 0.0,
            const float roughness = 0.045,
            const float reflectance = 0.5)
            : matColor{ colorToLinear(color) }
            , metallic{ metallic }
            , roughness{ std::clamp(roughness, minRoughness, maxRoughness) }
            , reflectance{ reflectance }
            , u_matColor{ materialType.getHandle("matColor") }
            , u_metallicRoughnessReflectance{ materialType.getHandle("metallicRoughnessReflectance") }
        {
        }

        inline void setUniforms() const override
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

        static MaterialType materialType;

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
