#include "Renderer.h"
#include "Cube.cpp"

namespace bae {
namespace MatTypes {
}; // namespace MatTypes

Renderer::~Renderer() noexcept
{
    pointLightUniforms.destroy();
}

void Renderer::init(Window* pWindow)
{
    this->pWindow = pWindow;
    width = pWindow->getWidth();
    height = pWindow->getHeight();

    // Create a window
    bgfx::PlatformData platformData = pWindow->getPlatformData();

    instance.initBgfx(platformData, width, height);
    PosColorVertex::init();
    PosTexNormalVertex::init();
    Materials::basic = matTypeManager.createMaterialType("basic", Materials::Basic::uniformInfoMap);

    bgfx::setViewClear(
        0,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x303030ff,
        1.0f,
        0);

    geoRegistry.create("cube", cubeVertices, cubeIndices);
    ModelLoader loader{ &geoRegistry };
    loader.loadModel("bunny", "/home/bruno/Documents/graphics/bae/assets/bunny.obj");

    pointLightUniforms.init();

    state = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW;
}

void Renderer::renderFrame(const float dt, Camera& camera, entt::DefaultRegistry& registry)
{
    bgfx::ViewId viewId{ 0 };

    // Set view 0 default viewport.
    bgfx::setViewRect(viewId, 0, 0, uint16_t(width), uint16_t(height));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(viewId);
    setupLighting<PointLightEmitter>(registry, pointLightUniforms);
    pointLightUniforms.set();

    auto stateCopy = state;
    camera.setViewTransform(viewId);
    bgfx::ProgramHandle program = Materials::basic.program;
    registry.view<Position, Geometry, Materials::Basic>().each(
        [dt, program, viewId, stateCopy](const auto&, const Position& pos, const auto& geo, const auto& mat) {
            glm::mat4 mtx = glm::translate(glm::identity<glm::mat4>(), pos);
            bgfx::setTransform(glm::value_ptr(mtx));

            geo.set(viewId);
            mat.setUniforms();
            bgfx::setState(stateCopy);
            bgfx::submit(viewId, program);
        });

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();
}
} // namespace bae
