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
#include "Instance.h"
#include "MaterialType.h"
#include "Mesh.h"
#include "MeshManager.h"
#include "Window.h"
#include "materials/Basic.h"
#include "utils/Vertex.h"

#if BX_PLATFORM_WINDOWS
#define SDL_MAIN_HANDLED
#endif // BX_PLATFORM_WINDOWS

namespace bae {
class Renderer {
public:
    Renderer() = default;
    ~Renderer() noexcept;

    void init(uint32_t width, uint32_t height);
    bool update(const float dt);
    void renderFrame(const float dt);

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::unique_ptr<bae::Window> m_pWindow = nullptr;
    uint64_t m_state = 0;

    Instance m_instance = Instance{};

    EventQueue m_eventQueue;
    WindowInputHandler windowInputHandler;

    Camera m_camera;
    Geometry m_geom = Geometry{};
    MeshManager<Materials::Basic> meshes;

    FPSControls m_cameraControls;
};
} // namespace bae
