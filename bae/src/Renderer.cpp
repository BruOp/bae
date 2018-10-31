#include "Renderer.h"
#include "Cube.cpp"
#include <iostream>

namespace bae
{
namespace MatTypes
{
MaterialType basic = MaterialType{"basic"};
UniformSetPrototype basicUniformPrototypeSet{{{"color", bgfx::UniformType::Vec4}}};

void init()
{
    basic.init();
}

void destroy()
{
    basic.destroy();
}
}; // namespace MatTypes

Renderer::~Renderer() noexcept
{
    bgfx::destroy(u_color);
    MatTypes::destroy();
}

void Renderer::init(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;

    startOffset = bx::getHPCounter();

    m_instance.initSDL();

    // Create a window
    m_pWindow = std::make_unique<bae::Window>(m_width, m_height);
    auto platformData = m_pWindow->getPlatformData();

    m_instance.initBgfx(platformData, m_width, m_height);
    Vertex::init();
    MatTypes::init();

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
    m_geom = Geometry{cubeVertices, cubeIndices};

    m_cameraControls = FPSControls{*m_camera};
    m_mesh = std::make_unique<Mesh>(m_geom, MatTypes::basic);
    u_color = bgfx::createUniform("color", bgfx::UniformType::Vec4);

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
    auto inputHandleResult = m_cameraControls.handleEvents(m_eventQueue);
    if (inputHandleResult == EventHandleResult::EVENT_RESULT_SHUTDOWN)
    {
        return false;
    }
    m_cameraControls.update();
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

    std::cout << m_camera->m_direction.x << m_camera->m_direction.y << m_camera->m_direction.z << std::endl;

    // glm::mat4 mtx{};
    //bgfx::setTransform(&mtx[0][0]);

    float color[4] = {0.0f, 1.0f, 0.0f, 1.0f};
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
