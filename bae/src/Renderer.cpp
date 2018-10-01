#include "Renderer.h"
#include <memory>

namespace bae
{
Renderer::~Renderer() noexcept
{
    // Shutdown application and glfw
    bgfx::shutdown();
    glfwTerminate();
}

void Renderer::init(uint32_t width, uint32_t height)
{
    _width = width;
    _height = height;

    if (!glfwInit())
    {
        throw std::runtime_error("Could not initialize glfw");
    }

    // Create a window
    _pWindow = std::make_unique<bae::Window>(_width, _height);
    auto platformData = _pWindow->getPlatformData();

    bgfx::setPlatformData(platformData);
    bgfx::Init init{};
    init.type = bgfx::RendererType::Vulkan;

    init.vendorId = BGFX_PCI_ID_NONE;
    init.deviceId = 0;
    init.resolution.width = width;
    init.resolution.height = height;
    init.resolution.reset = BGFX_RESET_VSYNC;

    bgfx::init(init);
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    bgfx::setViewClear(
        0,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x303030ff,
        1.0f,
        0);

    _lastTime = (float)glfwGetTime();
}

bool Renderer::update()
{
    glfwPollEvents();

    if (_pWindow->shouldClose())
    {
        return false;
    }

    float lastTime = 0;
    float dt;
    float time;
}

void Renderer::renderFrame()
{
    float time = (float)glfwGetTime();
    float dt = time - _lastTime;
    _lastTime = time;

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0, uint16_t(_width), uint16_t(_height));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

    // Use debug font to print information about this example.
    bgfx::dbgTextClear();

    const bgfx::Stats *stats = bgfx::getStats();
    bgfx::dbgTextPrintf(
        0,
        2,
        0x0f,
        "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters.",
        stats->width,
        stats->height,
        stats->textWidth,
        stats->textHeight);

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();
}
} // namespace bae
