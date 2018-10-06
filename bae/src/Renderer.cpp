#include "Renderer.h"

const std::vector<bae::Vertex> cubeVertices{
    {glm::vec3{-1.0f, 1.0f, 1.0f}, glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}},
    {glm::vec3{1.0f, 1.0f, 1.0f}, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}},
    {glm::vec3{-1.0f, -1.0f, 1.0f}, glm::vec4{1.0f, 1.0f, 0.0f, 1.0f}},
    {glm::vec3{1.0f, -1.0f, 1.0f}, glm::vec4{0.0f, 0.0f, 1.0f, 1.0f}},
    {glm::vec3{-1.0f, 1.0f, -1.0f}, glm::vec4{1.0f, 0.0f, 1.0f, 1.0f}},
    {glm::vec3{1.0f, 1.0f, -1.0f}, glm::vec4{0.0f, 1.0f, 1.0f, 1.0f}},
    {glm::vec3{-1.0f, -1.0f, -1.0f}, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}},
    {glm::vec3{1.0f, -1.0f, -1.0f}, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}},
};

const std::vector<uint16_t> cubeIndices{
    0,
    1,
    2, // 0
    1,
    3,
    2,
    4,
    6,
    5, // 2
    5,
    6,
    7,
    0,
    2,
    4, // 4
    4,
    2,
    6,
    1,
    5,
    3, // 6
    5,
    7,
    3,
    0,
    4,
    1, // 8
    4,
    5,
    1,
    2,
    3,
    6, // 10
    6,
    3,
    7,
};

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
    m_width = width;
    m_height = height;

    if (!glfwInit())
    {
        throw std::runtime_error("Could not initialize glfw");
    }

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

    m_lastTime = (float)glfwGetTime();
    m_geom = std::make_unique<Geometry>(cubeVertices, cubeIndices);
    m_mat = std::make_unique<MaterialType>("basic", "basic_vs.bin", "basic_fs.bin");
    m_mesh = std::make_unique<Mesh>(*m_geom, *m_mat);

    m_state = BGFX_STATE_DEFAULT;
    // 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW;
}

bool Renderer::update()
{
    glfwPollEvents();

    if (m_pWindow->shouldClose())
    {
        return false;
    }
    return true;
}

void Renderer::renderFrame()
{
    float time = (float)glfwGetTime();
    float dt = time - m_lastTime;
    m_lastTime = time;

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -35.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), float(m_width) / m_height, 0.1f, 100.0f);
    bgfx::setViewTransform(0, &view[0][0], &proj[0][0]);

    glm::mat4 mtx{};
    bgfx::setTransform(&mtx[0][0]);

    m_mesh->draw(0, m_state);

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();
}
} // namespace bae
