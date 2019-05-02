#pragma once
#include <stdexcept>

#include <SDL.h>
#include <SDL_syswm.h>
#include <bgfx/platform.h>

namespace bae
{
class Window
{
   public:
    Window();
    Window(const uint32_t width, const uint32_t height);

    ~Window();

    // No copying allowed!
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    // Moving is fine I guess
    Window(Window &&otherWindow);
    Window &operator=(Window &&otherWindow);

    void destroy();
    bool shouldClose(const SDL_Event &event) const;

    inline uint32_t getWidth() const { return m_width; }
    inline uint32_t getHeight() const { return m_height; }
    inline SDL_Window *getWindowPtr() const { return m_pWindow; }
    bgfx::PlatformData getPlatformData();

   private:
    SDL_Window *m_pWindow = nullptr;
    uint32_t m_width;
    uint32_t m_height;
};
}  // namespace bae
