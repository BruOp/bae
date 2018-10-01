#pragma once
#include <memory>

#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include "Window.h"

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
    uint32_t _width = 0;
    uint32_t _height = 0;
    std::unique_ptr<bae::Window> _pWindow = nullptr;
    float _lastTime = 0;
};
} // namespace bae
