#include "Instance.h"

namespace bae
{
Instance::~Instance() noexcept
{
    if (m_bfgx_initialized)
    {
        bgfx::shutdown();
        m_bfgx_initialized = false;
    }
    if (m_sdl_initialized)
    {
        SDL_Quit();
        m_sdl_initialized = false;
    }
}

void Instance::initSDL()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        throw std::runtime_error("Could not initialize SDL2");
    }
    SDL_SetRelativeMouseMode(SDL_TRUE);
    m_sdl_initialized = true;
}

void Instance::initBgfx(
    const bgfx::PlatformData platformData,
    const uint32_t width,
    const uint32_t height)
{
    bgfx::setPlatformData(platformData);
    bgfx::Init init{};
    init.type = bgfx::RendererType::OpenGL;

    init.vendorId = BGFX_PCI_ID_NONE;
    init.deviceId = 0;
    init.resolution.width = width;
    init.resolution.height = height;
    init.resolution.reset = BGFX_RESET_VSYNC;

    bgfx::init(init);
    m_bfgx_initialized = true;
    bgfx::setDebug(BGFX_DEBUG_TEXT);
}
} // namespace bae
