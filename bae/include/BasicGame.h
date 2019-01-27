#pragma once
#include <entt/entt.hpp>

#include "IGame.h"
#include "Renderer.h"
#include "utils/Common.h"

namespace bae {
class BasicGame : IGame {
public:
    BasicGame();
    ~BasicGame() noexcept;

    void start() override;
    bool update() override;

private:
    std::unique_ptr<bae::Window> pWindow = nullptr;

    EventQueue eventQueue;
    WindowInputHandler windowInputHandler;

    Renderer renderer;

    Camera camera;
    FPSControls cameraControls;

    entt::DefaultRegistry registry;
    uint64_t startOffset = 0;
    float lastTime = 0;

    inline void initSDL()
    {
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            throw std::runtime_error("Could not initialize SDL2");
        }
    }
};
}
