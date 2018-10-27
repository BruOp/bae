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
      m_right{-glm::cross(Camera::WorldUp, direction)},
      m_fov{glm::radians(fovInDegrees)},
      m_aspectRatio{(float)width / height},
      m_projection{calcProjection(m_fov, m_aspectRatio)}
{
}

void Camera::updateViewMatrix()
{
    glm::vec3 up = glm::normalize(glm::cross(m_right, m_direction));
    m_view = glm::lookAt(m_position, m_position + m_direction, up);
}

void Camera::setViewTransform(const bgfx::ViewId viewId) const
{
    bgfx::setViewTransform(viewId, &m_view[0][0], &m_projection[0][0]);
}

void Camera::moveAlongDirection(const bae::Direction &direction, const float movementSpeed)
{
    m_position += (movementSpeed * direction);
}

glm::mat4 Camera::calcProjection(
    const float &fov,
    const float aspectRatio)
{
    glm::mat4 proj = glm::perspective(fov, aspectRatio, 0.1f, 1000.0f);
    return proj;
}

const bae::Direction Camera::WorldUp{0.0f, 1.0f, 0.0f};
} // namespace bae
