#pragma once
#include <bx/timer.h>
#include <glm/glm.hpp>

#include "Geometric.h"

namespace bae {

    typedef uint32_t Entity;
    typedef uint32_t Parent;

inline float getTime(const uint64_t startOffset)
{
    return static_cast<float>((bx::getHPCounter() - startOffset) / static_cast<double>(bx::getHPFrequency()));
}

template <typename T>
inline T toGamma(const T value)
{
    return glm::pow(glm::abs(value), glm::vec3(1.0 / 2.2));
}

inline glm::vec4 colorToGamma(const glm::vec4& color)
{
    auto rgb = toGamma(glm::vec3{ color.x, color.y, color.z });
    return glm::vec4{ rgb.x, rgb.y, rgb.z, color.w };
}

template <typename T>
inline T toLinear(const T value)
{
    return glm::pow(glm::abs(value), glm::vec3(2.2));
}

inline glm::vec4 colorToLinear(const glm::vec4& color)
{
    auto rgb = toLinear(glm::vec3{ color.x, color.y, color.z });
    return glm::vec4{ rgb.x, rgb.y, rgb.z, color.w };
}
} // namespace bae
