#include "Instance.h"

namespace bae {
Instance::~Instance() noexcept
{
    if (bfgx_initialized) {
        bgfx::shutdown();
        bfgx_initialized = false;
    }
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
    init.resolution.reset = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X16;

    bgfx::init(init);
    bfgx_initialized = true;
    bgfx::setDebug(BGFX_DEBUG_TEXT);
}
} // namespace bae
