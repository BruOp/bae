#include "Camera.h"

namespace bae {

Camera::Camera(
    const glm::vec3 position,
    const glm::vec3 direction,
    const uint32_t width,
    const uint32_t height,
    const float fovInDegrees)
    : position{ position }
    , direction{ glm::normalize(direction) }
    , right{ -glm::cross(Camera::WorldUp, direction) }
    , fov{ glm::radians(fovInDegrees) }
    , aspectRatio{ (float)width / height }
    , projection{ calcProjection(fov, aspectRatio) }
    , view{}
{
}

void Camera::updateViewMatrix()
{
    glm::vec3 up = glm::normalize(glm::cross(right, direction));
    view = glm::lookAt(position, position + direction, up);
}

void Camera::moveAlongDirection(const bae::Direction& direction, const float movementSpeed)
{
    position += (movementSpeed * direction);
}

glm::mat4 Camera::calcProjection(
    const float& fov,
    const float aspectRatio)
{
    glm::mat4 proj = glm::perspective(fov, aspectRatio, 0.1f, 1000.0f);
    return proj;
}

const bae::Direction Camera::WorldUp{ 0.0f, 1.0f, 0.0f };
} // namespace bae
