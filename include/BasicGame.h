#pragma once
#include <entt/entt.hpp>

#include "IGame.h"
#include "Renderer.h"
#include "utils/Common.h"

namespace bae
{
struct WindowContext {
    ~WindowContext() noexcept
    {
        if (initialized) {
            SDL_Quit();
        }
    };

    inline void initSDL()
    {
        initialized = SDL_Init(SDL_INIT_EVERYTHING) >= 0;
        if (!initialized) {
            throw std::runtime_error("Could not initialize SDL2");
        }
    }

    bool initialized = false;
};

class BasicGame : IGame
{
   public:
    BasicGame() noexcept;

    void start() override;
    bool update() override;

   private:
    WindowContext windowContext;
    std::unique_ptr<bae::Window> pWindow = nullptr;

    EventQueue eventQueue;
    WindowInputHandler windowInputHandler;

    Renderer renderer;

    Camera camera;
    FPSControls cameraControls;

    entt::DefaultRegistry registry;
    uint64_t startOffset = 0;
    float lastTime = 0;
};
}  // namespace bae
