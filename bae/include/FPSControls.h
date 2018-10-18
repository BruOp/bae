#pragma once
#include <algorithm>
#include <iostream>
#include <optional>

#include "Camera.h"
#include "EventHandlers.h"

namespace bae
{
class FPSControls : public EventSubscriber
{
    enum KEYS
    {
        W = 1,
        S = 2,
        A = 4,
        D = 8
    };

  public:
    FPSControls() = default;
    FPSControls(
        Camera &camera,
        const float sensitivity = 0.002,
        const float movementSpeed = 0.1);

    EventHandleResult handleEvents(const EventQueue &eventQueue) override final;

    void update();

  private:
    Camera *m_pCamera = nullptr;
    glm::vec3 currentDirection = {0.0f, 0.0f, 0.0f};
    float m_sensitivity;
    float m_movementSpeed;
    float m_yaw = -glm::pi<float>() / 2.0f;
    float m_pitch = 0.0f;
    unsigned int keysPressed = 0;

    void handleKeydown(const SDL_Event &event);
    void handleKeyup(const SDL_Event &event);
    void handleMouseMovement(const SDL_Event &event);
};
} // namespace bae
