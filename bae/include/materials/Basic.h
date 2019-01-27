#pragma once
#include <glm/glm.hpp>

#include "MaterialType.h"
#include "Uniforms.h"

namespace bae {
namespace Materials {
    extern MaterialType basic;

    struct Basic {
        Vec4Uniform color;

        const static MaterialType* materialType;
        const static UniformInfoMap uniformInfoMap;

        Basic()
            : Basic{ glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f } } {};
        Basic(const glm::vec4& color)
            : color{ color, materialType->getHandle("color") }
        {
        }

        inline void setUniforms() const
        {
            color.setUniform();
        };
    };
}
}
