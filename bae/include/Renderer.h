#pragma once
#include <memory>
#include <stdexcept>

#include "SDL.h"
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/timer.h>
#include <glm/glm.hpp>

#include "Camera.h"
#include "EventHandlers.h"
#include "EventQueue.h"
#include "FPSControls.h"
#include "Mesh.h"
#include "Window.h"
#include "utils/Vertex.h"

#if BX_PLATFORM_WINDOWS
#define SDL_MAIN_HANDLED
#endif // BX_PLATFORM_WINDOWS

namespace bae
{
class Renderer
{
  public:
    Renderer() = default;
    ~Renderer() noexcept;

    void init(uint32_t width, uint32_t height);
    bool update();
    void renderFrame();

  private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::unique_ptr<bae::Window> m_pWindow = nullptr;
    uint64_t startOffset = 0;
    uint64_t m_lastTime = 0;
    uint64_t m_state = 0;

    EventQueue m_eventQueue;
    WindowInputHandler windowInputHandler;
    FPSControls cameraControls;

    std::unique_ptr<Camera> m_camera = nullptr;
    std::unique_ptr<Geometry> m_geom = nullptr;
    std::unique_ptr<MaterialType> m_mat = nullptr;
    std::unique_ptr<Mesh> m_mesh = nullptr;

    static float getTime(const uint64_t startOffset);
};
} // namespace bae
