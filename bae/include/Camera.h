#pragma once

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

#include "utils/Geometric.h"

namespace bae
{
class Camera
{
  public:
    Camera(
        const glm::vec3 position,
        const glm::vec3 direction,
        const uint32_t width,
        const uint32_t height,
        const float fovInDegrees = 45.0);

    void updateViewMatrix();

    void setViewTransform(const bgfx::ViewId viewId) const;
    // void updateProjection(const float &fov, const float aspectRatio);

    void moveAlongDirection(const Direction &direction, const float movementSpeed);

    friend class FPSControls;

  private:
    bae::Position m_position;
    bae::Direction m_direction;
    bae::Direction m_right;

    // Projection
    float m_fov;
    float m_aspectRatio;

    glm::mat4 m_view;
    glm::mat4 m_projection;

    static glm::mat4 calcProjection(
        const float &fov,
        const float aspectRatio);

    static const bae::Direction WorldUp;
};
} // namespace bae
