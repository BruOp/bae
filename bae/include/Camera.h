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

    void setViewTransform(const bgfx::ViewId viewId) const;
    // void updateProjection(const float &fov, const float aspectRatio);

    void moveAlongDirection(const Direction& direction, const float movementSpeed);

    friend class FPSControls;

    bae::Position m_position = Position(0.0f, 0.0f, 10.0f);
    bae::Direction m_direction = Direction(0.0f, 0.0f, -1.0f);
    bae::Direction m_right = Direction(1.0f, 0.0f, 0.0f);
    float m_fov;
    float m_aspectRatio;
    glm::mat4 m_projection;
    glm::mat4 m_view;

    // Projection

private:
    static glm::mat4 calcProjection(
        const float& fov,
        const float aspectRatio);

    static const bae::Direction WorldUp;
};
} // namespace bae
