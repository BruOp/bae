#pragma once
#include <bgfx/bgfx.h>

#include "Window.h"

namespace bae
{
// Usually the first to be created, last to be destroyed
class Instance
{
  public:
    Instance() = default;
    ~Instance() noexcept;
    Instance(const Instance &) = delete;
    Instance(Instance &&) = delete;
    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&) = delete;

    void initSDL();
    void initBgfx(
        const bgfx::PlatformData &platformData,
        const uint32_t width,
        const uint32_t height);

  private:
    bool m_sdl_initialized = false;
    bool m_bfgx_initialized = false;
};
} // namespace bae
