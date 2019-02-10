#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/fast_trigonometry.hpp>

#include "Camera.h"
#include "EventHandlers.h"
#include "utils/Constants.h"
#include "utils/Geometric.h"

namespace bae {
class FPSControls : public EventSubscriber {

public:
    FPSControls() = default;
    FPSControls(
        Camera& camera,
        const float sensitivity = 0.002,
        const float movementSpeed = 5.0);

    EventHandleResult handleEvents(const EventQueue& eventQueue) override final;

    void update(const float dt);

private:
    Camera* pCamera = nullptr;
    bae::Direction currentDirection = { 0.0f, 0.0f, 0.0f };
    float sensitivity;
    float movementSpeed;
    float yaw = -HALF_PI;
    float pitch = 0.0f;

    void handleKeydown(const SDL_Event& event);
    void handleKeyup(const SDL_Event& event);
    void handleMouseMovement(const SDL_Event& event);

    constexpr static float s_upperPitchLimit = HALF_PI - 0.1;
    constexpr static float s_lowerPitchLimit = -HALF_PI + 0.1;
};
} // namespace bae
