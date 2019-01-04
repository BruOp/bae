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

        inline void setUniforms() const
        {
            color.setUniform();
        };

        static Basic create(const glm::vec4& color)
        {
            return Basic{ { color, materialType->getHandle("color") } };
        };
    };
}
}
