#pragma once

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

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

    void setViewTransform(const bgfx::ViewId viewId) const;
    // void updateProjection(const float &fov, const float aspectRatio);

  private:
    glm::vec3 m_position;
    glm::vec3 m_direction;
    glm::vec3 m_right;
    // Projection
    float m_fov;
    float m_aspectRatio;
    glm::mat4 m_projection;

    static glm::mat4 calcProjection(
        const float &fov,
        const float aspectRatio);

    static const glm::vec3 up;
};
} // namespace bae
