#include "FPSControls.h"

namespace bae
{
FPSControls::FPSControls(
    Camera &camera,
    const float sensitivity,
    const float movementSpeed)
    : m_pCamera{&camera},
      m_sensitivity{sensitivity},
      m_movementSpeed{movementSpeed} {};

EventHandleResult FPSControls::handleEvents(
    const EventQueue &eventQueue)
{
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
    for (const auto &event : eventQueue)
    {
        if (event.type == SDL_KEYDOWN)
        {
            direction += handleKeydown(event);
        }
        if (event.type == SDL_MOUSEMOTION)
        {
            handleMouseMovement(event);
        }
    }
    if (glm::length(direction) != 0.0f)
    {
        m_pCamera->moveAlongDirection(direction, m_movementSpeed);
    }

    return EventHandleResult::EVENT_RESULT_CONTINUE;
}

glm::vec3 FPSControls::handleKeydown(const SDL_Event &event) const
{
    switch (event.key.keysym.sym)
    {
    case SDLK_w:
        return m_pCamera->m_direction;
    case SDLK_s:
        return -m_pCamera->m_direction;
    case SDLK_d:
        return m_pCamera->m_right;
    case SDLK_a:
        return -m_pCamera->m_right;
    default:
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
}

void FPSControls::handleMouseMovement(const SDL_Event &event)
{
    m_pCamera->m_yaw += event.motion.xrel * m_sensitivity;
    m_pCamera->m_pitch -= event.motion.yrel * m_sensitivity;
    glm::vec3 front{
        glm::cos(m_pCamera->m_yaw) * glm::cos(m_pCamera->m_pitch),
        glm::sin(m_pCamera->m_pitch),
        glm::cos(m_pCamera->m_pitch) * glm::sin(m_pCamera->m_yaw)};
    front = glm::normalize(front);
    m_pCamera->m_direction = glm::normalize(front);
    m_pCamera->m_right = glm::cross(m_pCamera->m_direction, glm::vec3{0.0f, 1.0f, 0.0f});
}
} // namespace bae
