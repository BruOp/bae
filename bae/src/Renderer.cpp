#include "Renderer.h"
#include "Cube.cpp"

namespace bae {
namespace MatTypes {
}; // namespace MatTypes

Renderer::~Renderer() noexcept
{
    Materials::basic.destroy();
}

void Renderer::init(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;

    m_instance.initSDL();

    // Create a window
    m_pWindow = std::make_unique<bae::Window>(m_width, m_height);
    bgfx::PlatformData platformData = m_pWindow->getPlatformData();

    m_instance.initBgfx(platformData, m_width, m_height);
    Vertex::init();
    Materials::basic.init();

    bgfx::setViewClear(
        0,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x303030ff,
        1.0f,
        0);

    m_camera = Camera{
        glm::vec3{ 5.0f, 2.0f, 10.0f },
        glm::vec3{ 0.0f, 0.0f, -1.0f },
        m_width,
        m_height,
        60.0f
    };

    geoRegistry.create("cube", cubeVertices, cubeIndices);

    m_cameraControls = FPSControls{ m_camera };

    m_state = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW;
}

bool Renderer::update(const float dt)
{
    auto res = m_eventQueue.pump();
    auto eventhandleResult = windowInputHandler.handleEvents(m_eventQueue);
    if (eventhandleResult == EventHandleResult::EVENT_RESULT_SHUTDOWN) {
        return false;
    }
    auto inputHandleResult = m_cameraControls.handleEvents(m_eventQueue);
    if (inputHandleResult == EventHandleResult::EVENT_RESULT_SHUTDOWN) {
        return false;
    }
    m_cameraControls.update(dt);
    return true;
}

void Renderer::renderFrame(const float dt, entt::DefaultRegistry& registry)
{
    bgfx::ViewId viewId{ 0 };

    // Set view 0 default viewport.
    bgfx::setViewRect(viewId, 0, 0, uint16_t(m_width), uint16_t(m_height));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(viewId);

    m_camera.setViewTransform(viewId);
    auto state = m_state;
    bgfx::ProgramHandle program = Materials::basic.getProgram();
    registry.view<Position, Geometry, Materials::Basic>().each(
        [dt, program, viewId, state](const auto&, const Position& pos, const auto& geo, const auto& mat) {
            glm::mat4 mtx = glm::translate(glm::identity<glm::mat4>(), pos);
            bgfx::setTransform(glm::value_ptr(mtx));

            geo.set(viewId);
            mat.setUniforms();
            bgfx::setState(state);
            bgfx::submit(viewId, program);
        });

    // meshes.draw(viewId, m_state);

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();
}
} // namespace bae
