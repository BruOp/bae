#pragma once

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

#include "utils/Geometric.h"

namespace bae {
class Camera {
public:
    Camera() = default;
    Camera(
        const Position position,
        const Direction direction,
        const uint32_t width,
        const uint32_t height,
        const float fovInDegrees = 45.0);

    void updateViewMatrix();

    // void updateProjection(const float &fov, const float aspectRatio);

    void moveAlongDirection(const Direction& direction, const float movementSpeed);

    friend class FPSControls;

    bae::Position position = Position(0.0f, 0.0f, 10.0f);
    bae::Direction direction = Direction(0.0f, 0.0f, -1.0f);
    bae::Direction right = Direction(1.0f, 0.0f, 0.0f);
    float fov;
    float aspectRatio;
    glm::mat4 projection;
    glm::mat4 view;

    // Projection

private:
    static glm::mat4 calcProjection(
        const float& fov,
        const float aspectRatio);

    static const bae::Direction WorldUp;
};
} // namespace bae
