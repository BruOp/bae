#pragma once
#include "MaterialType.h"
#include "Uniforms.h"
#include <glm/glm.hpp>

namespace bae {
namespace Materials {
    extern MaterialType basic;

    class Basic {
    public:
        Basic() = default;
        Basic(const glm::vec4& color)
            : color{ "color", color } {};

        Basic(const Basic&) = delete;
        Basic& operator=(const Basic&) = delete;
        Basic(Basic&&) = default;
        Basic& operator=(Basic&&) = default;

        inline void setUniforms() const
        {
            color.setUniform();
        };

        inline void destroy()
        {
            color.destroy();
        };

        const static MaterialType* materialType;

    private:
        Vec4Uniform color = Vec4Uniform{ "color", glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f } };
    };
}
}
