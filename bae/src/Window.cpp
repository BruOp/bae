#include "Window.h"

namespace bae
{
Window::Window() : _pWindow(nullptr), _width(0), _height(0)
{
}

Window::Window(const uint32_t width, const uint32_t height)
    : _pWindow(nullptr),
      _width(width),
      _height(height)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    _pWindow = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
}

Window::Window(Window &&otherWindow)
{
    _width = otherWindow._width;
    _height = otherWindow._height;
    _pWindow = otherWindow._pWindow;
    otherWindow._pWindow = nullptr;
}

Window &Window::operator=(Window &&otherWindow)
{
    if (this != &otherWindow)
    {
        _width = otherWindow._width;
        _height = otherWindow._height;
        _pWindow = otherWindow._pWindow;
        otherWindow._pWindow = nullptr;
    }
    return *this;
}

Window::~Window()
{
    destroy();
}

void Window::destroy()
{
    if (_pWindow != nullptr)
    {
        glfwDestroyWindow(_pWindow);
    }
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(_pWindow);
}

void *Window::getNativeHandle()
{
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    return (void *)(uintptr_t)glfwGetX11Window(_pWindow);
#elif BX_PLATFORM_OSX
    return glfwGetCocoaWindow(_pWindow);
#elif BX_PLATFORM_WINDOWS
    return glfwGetWin32Window(_pWindow);
#endif // BX_PLATFORM_
};

bgfx::PlatformData Window::getPlatformData()
{
    bgfx::PlatformData pd;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    pd.ndt = glfwGetX11Display();
#elif BX_PLATFORM_OSX
    pd.ndt = NULL;
#elif BX_PLATFORM_WINDOWS
    pd.ndt = NULL;
#endif // BX_PLATFORM_WINDOWS
    pd.nwh = getNativeHandle();
    pd.context = NULL;
    pd.backBuffer = NULL;
    pd.backBufferDS = NULL;
    return pd;
}

void Window::pollWindowEvents()
{
    glfwPollEvents();
}
} // namespace bae
