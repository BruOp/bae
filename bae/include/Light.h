#pragma once
#include <glm/glm.hpp>

namespace bae {
struct PointLightEmitter {
    glm::vec3 color = { 1.0f, 1.0f, 1.0f };
    float intensity = 10.0f;
};
} // namespace bae
