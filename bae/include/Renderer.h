#pragma once
#include <memory>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "Mesh.h"
#include "Window.h"
#include "utils/Vertex.h"

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
    float m_lastTime = 0;
    uint64_t m_state;

    std::unique_ptr<Geometry> m_geom = nullptr;
    std::unique_ptr<MaterialType> m_mat = nullptr;
    std::unique_ptr<Mesh> m_mesh = nullptr;
};
} // namespace bae
