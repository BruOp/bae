#include "Renderer.h"

#if BX_PLATFORM_WINDOWS
#define SDL_MAIN_HANDLED
#endif  // BX_PLATFORM_WINDOWS

namespace bae
{
namespace MatTypes
{
};  // namespace MatTypes

Renderer::~Renderer() noexcept
{
    toneMappingPass.destroy();
    bgfx::destroy(pbrFramebuffer);
    
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
    initializeVertexDecls();

    matTypeManager.registerMaterialType<Materials::Basic>();
    matTypeManager.registerMaterialType<Materials::Lambertian>();
    matTypeManager.registerMaterialType<Materials::Physical>();
    matTypeManager.registerMaterialType<Materials::TexturedBasic>();
    matTypeManager.registerMaterialType<Materials::TexturedPhysical>();

    pointLightUniforms.init();
    sceneUniforms.init();

    pbrFbTextures[0] = bgfx::createTexture2D(
        uint16_t(width),
        uint16_t(height),
        false,
        1,
        bgfx::TextureFormat::RGBA16F,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
    );
    
    const uint64_t textureFlags = BGFX_TEXTURE_RT_WRITE_ONLY;

    bgfx::TextureFormat::Enum depthFormat;
    if (bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D16, textureFlags)) {
        depthFormat = bgfx::TextureFormat::D16;
    }
    else if (bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, textureFlags)) {
        depthFormat = bgfx::TextureFormat::D24S8;
    }
    else {
        depthFormat = bgfx::TextureFormat::D32;
    }
    
    pbrFbTextures[1] = bgfx::createTexture2D(
        uint16_t(width),
        uint16_t(height),
        false,
        1,
        depthFormat,
        textureFlags
    );

    pbrFramebuffer = bgfx::createFrameBuffer(pbrFbTextures.size(), pbrFbTextures.data(), true);

    toneMappingParams.width = width;
    toneMappingParams.width = height;
    toneMappingParams.originBottomLeft = bgfx::getCaps()->originBottomLeft;
    
    toneMappingPass.init();
    
    state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW; //| BGFX_STATE_WRITE_Z | ;
}

void Renderer::renderFrame(const float dt, const float _time, Camera& camera, entt::DefaultRegistry& registry)
{
    bgfx::setViewName(meshPass, "Meshes");
    bgfx::setViewClear(meshPass, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
    bgfx::setViewFrameBuffer(meshPass, pbrFramebuffer);

    // Set view 0 default viewport.
    bgfx::setViewRect(meshPass, 0, 0, uint16_t(width), uint16_t(height));

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(meshPass);
    setupLighting<PointLightEmitter>(registry, pointLightUniforms);
    pointLightUniforms.set();

    sceneUniforms.setCamera(camera, meshPass);

    renderMaterialCollection<Materials::Basic>(registry, meshPass, state);
    renderMaterialCollection<Materials::Lambertian>(registry, meshPass, state);
    renderMaterialCollection<Materials::Physical>(registry, meshPass, state);
    renderMaterialCollection<Materials::TexturedBasic>(registry, meshPass, state);
    renderMaterialCollection<Materials::TexturedPhysical>(registry, meshPass, state);

    toneMappingPass.render(pbrFbTextures[0], toneMappingParams, dt);

    // Advance to next frame. Rendering thread will be kicked to
    // process submitted rendering primitives.
    bgfx::frame();
}
}  // namespace bae
