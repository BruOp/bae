#include "FPSControls.h"

namespace bae {
FPSControls::FPSControls(
    Camera& camera,
    const float sensitivity,
    const float movementSpeed)
    : pCamera{ &camera }
    , sensitivity{ sensitivity }
    , movementSpeed{ movementSpeed } {};

EventHandleResult FPSControls::handleEvents(
    const EventQueue& eventQueue)
{
    for (const auto& event : eventQueue) {
        if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                return EventHandleResult::EVENT_RESULT_SHUTDOWN;
            }
            handleKeydown(event);
        } else if (event.type == SDL_KEYUP && event.key.repeat == 0) {
            handleKeyup(event);
        } else if (event.type == SDL_MOUSEMOTION) {
            handleMouseMovement(event);
        }
    }
    return EventHandleResult::EVENT_RESULT_CONTINUE;
}

void FPSControls::handleKeydown(const SDL_Event& event)
{
    switch (event.key.keysym.sym) {
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
    case SDLK_SPACE:
        currentDirection.y += 0.1f;
        return;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
        currentDirection.y -= 0.1f;
        return;
    }
}

void FPSControls::handleKeyup(const SDL_Event& event)
{
    switch (event.key.keysym.sym) {
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
    case SDLK_SPACE:
        currentDirection.y -= 0.1f;
        return;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
        currentDirection.y += 0.1f;
        return;
    }
}

void FPSControls::handleMouseMovement(const SDL_Event& event)
{
    yaw = glm::wrapAngle(yaw + event.motion.xrel * sensitivity);

    pitch = glm::clamp<float>(
        pitch - event.motion.yrel * sensitivity,
        s_lowerPitchLimit,
        s_upperPitchLimit);
}

void FPSControls::update(const float dt)
{
    glm::vec3 front{
        glm::cos(yaw) * glm::cos(pitch),
        glm::sin(pitch),
        glm::cos(pitch) * glm::sin(yaw)
    };
    pCamera->direction = glm::normalize(front);
    pCamera->right = crossAndNormalize(pCamera->direction, Camera::WorldUp);
    pCamera->updateViewMatrix();

    if (glm::length(currentDirection) != 0) {
        glm::vec3 movementVector = glm::normalize(
            currentDirection.z * pCamera->direction + currentDirection.x * pCamera->right + currentDirection.y * pCamera->WorldUp);
        pCamera->moveAlongDirection(movementVector, dt * movementSpeed);
    }
}
} // namespace bae
