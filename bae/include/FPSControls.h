#pragma once
#include "Camera.h"
#include "EventHandlers.h"

namespace bae
{
class FPSControls : public EventSubscriber
{
  public:
    FPSControls() = default;
    FPSControls(
        Camera &camera,
        const float sensitivity = 0.002,
        const float movementSpeed = 0.1);

    EventHandleResult handleEvents(const EventQueue &eventQueue) override final;

  private:
    Camera *m_pCamera = nullptr;
    float m_sensitivity;
    float m_movementSpeed;

    glm::vec3 handleKeydown(const SDL_Event &event) const;
    void handleMouseMovement(const SDL_Event &event);
};
} // namespace bae
