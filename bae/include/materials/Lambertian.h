#pragma once
#include <glm/glm.hpp>

#include "MaterialType.h"
#include "Uniforms.h"

namespace bae {
namespace Materials {
    extern MaterialType lambertian;

    struct Lambertian {
        Vec4Uniform color;

        const static MaterialType* materialType;
        const static UniformInfoMap uniformInfoMap;

        Lambertian()
            : Lambertian{ glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f } } {};
        Lambertian(const glm::vec4& color)
            : color{ color, materialType->getHandle("matColor") }
        {
        }

        inline void setUniforms() const
        {
            color.setUniform();
        };
    };
}
}
