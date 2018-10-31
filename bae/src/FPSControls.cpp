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
    for (const auto &event : eventQueue)
    {
        if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
        {
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                return EventHandleResult::EVENT_RESULT_SHUTDOWN;
            }
            handleKeydown(event);
        }
        else if (event.type == SDL_KEYUP && event.key.repeat == 0)
        {
            handleKeyup(event);
        }
        else if (event.type == SDL_MOUSEMOTION)
        {
            handleMouseMovement(event);
        }
    }
    return EventHandleResult::EVENT_RESULT_CONTINUE;
}

void FPSControls::handleKeydown(const SDL_Event &event)
{
    switch (event.key.keysym.sym)
    {
    case SDLK_w:
        currentDirection.z += 1.0f;
        return;
    case SDLK_s:
        currentDirection.z -= 1.0f;
        return;
    case SDLK_d:
        currentDirection.x += 1.0f;
        return;
    case SDLK_a:
        currentDirection.x -= 1.0f;
        return;
    }
}

void FPSControls::handleKeyup(const SDL_Event &event)
{
    switch (event.key.keysym.sym)
    {
    case SDLK_w:
        currentDirection.z -= 1.0f;
        return;
    case SDLK_s:
        currentDirection.z += 1.0f;
        return;
    case SDLK_d:
        currentDirection.x -= 1.0f;
        return;
    case SDLK_a:
        currentDirection.x += 1.0f;
        return;
    }
}

void FPSControls::handleMouseMovement(const SDL_Event &event)
{
    m_yaw = glm::wrapAngle(m_yaw + event.motion.xrel * m_sensitivity);

    m_pitch = glm::clamp<float>(
        m_pitch - event.motion.yrel * m_sensitivity,
        s_lowerPitchLimit,
        s_upperPitchLimit);
}

void FPSControls::update()
{
    glm::vec3 front{
        glm::cos(m_yaw) * glm::cos(m_pitch),
        glm::sin(m_pitch),
        glm::cos(m_pitch) * glm::sin(m_yaw)};
    m_pCamera->m_direction = glm::normalize(front);
    m_pCamera->m_right = crossAndNormalize(m_pCamera->m_direction, Camera::WorldUp);
    m_pCamera->updateViewMatrix();

    if (glm::length(currentDirection) != 0)
    {
        glm::vec3 movementVector = glm::normalize(
            currentDirection.z * m_pCamera->m_direction +
            currentDirection.x * m_pCamera->m_right);
        m_pCamera->moveAlongDirection(movementVector, m_movementSpeed);
    }
}
} // namespace bae
