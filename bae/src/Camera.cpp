#include "Camera.h"

namespace bae
{

Camera::Camera(
    const glm::vec3 position,
    const glm::vec3 direction,
    const uint32_t width,
    const uint32_t height,
    const float fovInDegrees)
    : m_position{position},
      m_direction{glm::normalize(direction)},
      m_right{-glm::cross(Camera::up, direction)},
      m_fov{glm::radians(fovInDegrees)},
      m_aspectRatio{(float)width / height},
      m_projection{calcProjection(m_fov, m_aspectRatio)}
{
}

void Camera::setViewTransform(const bgfx::ViewId viewId) const
{
    glm::vec3 up = glm::normalize(glm::cross(m_right, m_direction));
    glm::mat4 view = glm::lookAt(m_position, m_position + m_direction, up);
    bgfx::setViewTransform(viewId, &view[0][0], &m_projection[0][0]);
}

glm::mat4 Camera::calcProjection(
    const float &fov,
    const float aspectRatio)
{
    glm::mat4 proj = glm::perspective(fov, aspectRatio, 0.1f, 1000.0f);
    return proj;
}

const glm::vec3 Camera::up{0.0f, 1.0f, 0.0f};
} // namespace bae