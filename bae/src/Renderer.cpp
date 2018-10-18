#include "Renderer.h"

const std::vector<bae::Vertex> cubeVertices{
    {glm::vec3{-1.0f, 1.0f, 1.0f}, 0xff000000},
    {glm::vec3{1.0f, 1.0f, 1.0f}, 0xff0000ff},
    {glm::vec3{-1.0f, -1.0f, 1.0f}, 0xff00ff00},
    {glm::vec3{1.0f, -1.0f, 1.0f}, 0xffff0000},
    {glm::vec3{-1.0f, 1.0f, -1.0f}, 0xff00ffff},
    {glm::vec3{1.0f, 1.0f, -1.0f}, 0xffff00ff},
    {glm::vec3{-1.0f, -1.0f, -1.0f}, 0xffffff00},
    {glm::vec3{1.0f, -1.0f, -1.0f}, 0xffffffff},
};

const std::vector<uint16_t> cubeIndices{
    0,
    1,
    2, // 0
    1,
    3,
    2, // 1
    4,
    6,
    5, // 2
    5,
    6,
    7, // 3
    0,
    2,
    4, // 4
    4,
    2,
    6, // 5
    1,
    5,
    3, // 6
    5,
    7,
    3, // 7
    0,
    4,
    1, // 8
    4,
    5,
    1, // 9
    2,
    3,
    6, // 10
    6,
    3,
    7, // 11
};

namespace bae
{
Renderer::~Renderer() noexcept
{
    m_mesh = nullptr;
    m_mat = nullptr;
    m_geom = nullptr;
    // Shutdown application and SDL
    bgfx::shutdown();
    SDL_Quit();
}

void Renderer::init(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;

    startOffset = bx::getHPCounter();

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        throw std::runtime_error("Could not initialize SDL2");
    }
    // SDL_SetRelativeMouseMode(SDL_TRUE);

    // Create a window
    m_pWindow = std::make_unique<bae::Window>(m_width, m_height);
    auto platformData = m_pWindow->getPlatformData();

    bgfx::setPlatformData(platformData);
    bgfx::Init init{};
    init.type = bgfx::RendererType::OpenGL;

    init.vendorId = BGFX_PCI_ID_NONE;
    init.deviceId = 0;
    init.resolution.width = m_width;
    init.resolution.height = m_height;
    init.resolution.reset = BGFX_RESET_VSYNC;

    bgfx::init(init);
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    Vertex::init();

    bgfx::setViewClear(
        0,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x303030ff,
        1.0f,
        0);

    m_camera = std::make_unique<Camera>(
        glm::vec3{5.0f, 2.0f, 10.0f},
        glm::vec3{0.0f, 0.0f, -1.0f},
        m_width,
        m_height,
        60.0f);

    m_lastTime = getTime(startOffset);
    m_geom = std::make_unique<Geometry>(cubeVertices, cubeIndices);
    m_mat = std::make_unique<MaterialType>("basic", "basic_vs.bin", "basic_fs.bin");
    m_mesh = std::make_unique<Mesh>(*m_geom, *m_mat);

    cameraControls = FPSControls{*m_camera};

    m_state =
        0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW;
}

bool Renderer::update()
{
    auto res = m_eventQueue.pump();
    auto eventhandleResult = windowInputHandler.handleEvents(m_eventQueue);
    if (eventhandleResult == EventHandleResult::EVENT_RESULT_SHUTDOWN)
    {
        return false;
    }
    auto inputHandleResult = cameraControls.handleEvents(m_eventQueue);
    cameraControls.update();
    return true;
}

void Renderer::renderFrame()
{
    bgfx::ViewId viewId{0};
    float time_ = getTime(startOffset);
    float dt = time_ - m_lastTime;
    m_lastTime = time_;

    // Set view 0 default viewport.
    bgfx::setViewRect(viewId, 0, 0, uint16_t(m_width), uint16_t(m_height));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(viewId);

    m_camera->setViewTransform(viewId);

    glm::mat4 mtx{};
    //bgfx::setTransform(&mtx[0][0]);

    m_mesh->draw(viewId, m_state);

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();
}

float Renderer::getTime(const uint64_t startOffset)
{
    return (float)((bx::getHPCounter() - startOffset) / double(bx::getHPFrequency()));
}
} // namespace bae
