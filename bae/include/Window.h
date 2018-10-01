#pragma once
#include <exception>

#include <bgfx/platform.h>
#include <GLFW/glfw3.h>

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#elif BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif
#include <GLFW/glfw3native.h>

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
    bool shouldClose() const;

    inline uint32_t getWidth() const { return _width; }
    inline uint32_t getHeight() const { return _height; }
    inline GLFWwindow *getWindowPtr() const { return _pWindow; }
    bgfx::PlatformData getPlatformData();

    void pollWindowEvents();

  private:
    GLFWwindow *_pWindow;
    uint32_t _width;
    uint32_t _height;

    void *getNativeHandle();
};
} // namespace bae
