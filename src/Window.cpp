#include "Window.h"

namespace bae
{
Window::Window() : m_pWindow(nullptr), m_width(0), m_height(0) {}

Window::Window(const uint32_t width, const uint32_t height) : m_width(width), m_height(height), m_pWindow(nullptr)
{
    m_pWindow = SDL_CreateWindow(
        "Bruno's Awful Engine",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        m_width,
        m_height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
}

Window::Window(Window &&otherWindow)
{
    m_width = otherWindow.m_width;
    m_height = otherWindow.m_height;
    m_pWindow = otherWindow.m_pWindow;
    otherWindow.m_pWindow = nullptr;
}

Window &Window::operator=(Window &&otherWindow)
{
    if (this != &otherWindow) {
        m_width = otherWindow.m_width;
        m_height = otherWindow.m_height;
        m_pWindow = otherWindow.m_pWindow;
        otherWindow.m_pWindow = nullptr;
    }
    return *this;
}

Window::~Window() { destroy(); }

void Window::destroy()
{
    if (m_pWindow != nullptr) {
        SDL_DestroyWindow(m_pWindow);
    }
}

bool Window::shouldClose(const SDL_Event &event) const { return event.type == SDL_QUIT; }

bgfx::PlatformData Window::getPlatformData()
{
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    if (SDL_GetWindowWMInfo(m_pWindow, &wmi) != SDL_TRUE) {
        const std::string error = SDL_GetError();
        throw std::runtime_error(error);
    }
    auto &info = wmi.info;
    bgfx::PlatformData pd;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    pd.ndt = info.x11.display;
    pd.nwh = (void *)(uintptr_t)info.x11.window;
#elif BX_PLATFORM_OSX
    pd.ndt = NULL;
    pd.nwh = info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
    pd.ndt = NULL;
    pd.nwh = info.win.window;
#endif  // BX_PLATFORM_WINDOWS
    pd.context = NULL;
    pd.backBuffer = NULL;
    pd.backBufferDS = NULL;
    return pd;
}
}  // namespace bae
