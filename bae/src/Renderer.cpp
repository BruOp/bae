#include "Renderer.h"
#include "Cube.cpp"

namespace bae {
namespace MatTypes {
}; // namespace MatTypes

Renderer::~Renderer() noexcept
{
    sceneUniforms.destroy();
    pointLightUniforms.destroy();
}

void Renderer::init(Window* pWindow) noexcept
{
    this->pWindow = pWindow;
    width = pWindow->getWidth();
    height = pWindow->getHeight();

    // Create a window
    bgfx::PlatformData platformData = pWindow->getPlatformData();

    instance.initBgfx(platformData, width, height);
    PosColorVertex::init();
    PosTexNormalVertex::init();

    matTypeManager.registerMaterialType<Materials::Basic>();
    matTypeManager.registerMaterialType<Materials::Lambertian>();
    matTypeManager.registerMaterialType<Materials::Physical>();

    bgfx::setViewClear(
        0,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x303030ff,
        1.0f,
        0);

    pointLightUniforms.init();
    sceneUniforms.init();

    geoRegistry.create("cube", cubeVertices, cubeIndices);
    ModelLoader loader{ &geoRegistry };

    std::string asset_dir = ASSETS_DIR;
    std::string bunny_path = asset_dir + "/bunny.obj";
    loader.loadModel("bunny", bunny_path);

    std::string material_sphere_path = asset_dir + "/material_sphere.obj";
    loader.loadModel("materialSphere", material_sphere_path);

    state = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW;
}

void Renderer::renderFrame(const float dt, const float _time, Camera& camera, entt::DefaultRegistry& registry)
{
    bgfx::ViewId viewId{ 0 };

    // Set view 0 default viewport.
    bgfx::setViewRect(viewId, 0, 0, uint16_t(width), uint16_t(height));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(viewId);
    setupLighting<PointLightEmitter>(registry, pointLightUniforms);
    pointLightUniforms.set();

    sceneUniforms.setCamera(camera, viewId);

    renderMaterialCollection<Materials::Basic>(registry, viewId, state);
    renderMaterialCollection<Materials::Lambertian>(registry, viewId, state);
    renderMaterialCollection<Materials::Physical>(registry, viewId, state);
    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();
}
} // namespace bae
