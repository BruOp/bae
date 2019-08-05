#include <array>
#include "bgfx_utils.h"
#include "common.h"
#include "imgui/imgui.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "bae/Offscreen.h"
#include "bae/Tonemapping.h"
#include "bae/PhysicallyBasedScene.h"

namespace example
{
#define SAMPLER_POINT_CLAMP (BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP)

    static float s_texelHalf = 0.0f;

    std::vector<glm::vec3> LIGHT_COLORS = {
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 0.1f, 0.1f },
        { 0.1f, 1.0f, 0.1f },
        { 0.1f, 0.1f, 1.0f },
        { 1.0f, 0.1f, 1.0f },
        { 1.0f, 1.0f, 0.1f },
        { 0.1f, 1.0f, 1.0f },
    };

    class BrdfLutCreator
    {
    public:
        void init()
        {
            const std::string brdfLutShader = "cs_brdf_lut";
            brdfLUTProgram = loadProgram(brdfLutShader.c_str(), nullptr);

            uint64_t lutFlags = BGFX_TEXTURE_COMPUTE_WRITE | SAMPLER_POINT_CLAMP;
            brdfLUT = bgfx::createTexture2D(width, width, false, 1, bgfx::TextureFormat::RG16F, lutFlags);
            bgfx::setName(brdfLUT, "Smith BRDF LUT");
        }

        bgfx::TextureHandle getLUT()
        {
            return brdfLUT;
        }

        void renderLUT(bgfx::ViewId view)
        {
            const uint16_t threadCount = 16u;
            bgfx::setViewName(view, "BRDF LUT creation pass");

            bgfx::setImage(0, brdfLUT, 0, bgfx::Access::Write, bgfx::TextureFormat::RG16F);
            bgfx::dispatch(view, brdfLUTProgram, width / threadCount, width / threadCount, 1);
            rendered = true;
        }

        void destroy()
        {
            bgfx::destroy(brdfLUTProgram);
            if (destroyTextureOnClose) {
                bgfx::destroy(brdfLUT);
            }
        }

        uint16_t width = 128u;

        // PBR IRB Textures and LUT
        bgfx::TextureHandle brdfLUT;
        bgfx::ProgramHandle brdfLUTProgram = BGFX_INVALID_HANDLE;

        bool rendered = false;
        bool destroyTextureOnClose = true;
    };

    class CubeMapFilterer
    {
    public:
        void init()
        {
            std::string shader = "cs_prefilter_env_map";
            preFilteringProgram = loadProgram(shader.c_str(), nullptr);
            shader = "cs_irradiance";
            irradianceProgram = loadProgram(shader.c_str(), nullptr);

            uint64_t flags = BGFX_TEXTURE_COMPUTE_WRITE;
            u_sourceCubeMap = bgfx::createUniform("u_source", bgfx::UniformType::Sampler);
            u_params = bgfx::createUniform("u_params", bgfx::UniformType::Vec4);
            filteredCubeMap = bgfx::createTextureCube(width, true, 1, bgfx::TextureFormat::RGBA16F, flags);
            irradianceMap = bgfx::createTextureCube(irradianceMapSize, false, 1, bgfx::TextureFormat::RGBA16F, flags);
            bgfx::setName(filteredCubeMap, "Prefilter Env Map");
        }

        bgfx::TextureHandle getPrefilteredMap()
        {
            return filteredCubeMap;
        }

        bgfx::TextureHandle getIrradianceMap()
        {
            return irradianceMap;
        }

        void render(bgfx::ViewId view)
        {
            const uint16_t threadCount = 8u;
            bgfx::setViewName(view, "Env Map Filtering Pass");

            float maxMipLevel = bx::log2(float(width));
            for (float mipLevel = 0; mipLevel <= maxMipLevel; ++mipLevel)
            {
                uint16_t mipWidth = width / uint16_t(bx::pow(2.0f, mipLevel));
                float roughness = mipLevel / maxMipLevel;
                float params[] = { roughness, float(mipLevel), float(width), 0.0f };
                bgfx::setUniform(u_params, params);
                bgfx::setTexture(0, u_sourceCubeMap, sourceCubeMap);
                bgfx::setImage(1, filteredCubeMap, uint8_t(mipLevel), bgfx::Access::Write, bgfx::TextureFormat::RGBA16F);
                bgfx::dispatch(view, preFilteringProgram, mipWidth / threadCount, mipWidth / threadCount, 1);
            }


            bgfx::setTexture(0, u_sourceCubeMap, sourceCubeMap);
            bgfx::setImage(1, irradianceMap, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA16F);
            bgfx::dispatch(view, irradianceProgram, irradianceMapSize / threadCount, irradianceMapSize / threadCount, 1);

            rendered = true;
        }

        void destroy()
        {
            bgfx::destroy(preFilteringProgram);
            bgfx::destroy(u_sourceCubeMap);
            bgfx::destroy(u_params);

            if (destroyTextureOnClose) {
                bgfx::destroy(sourceCubeMap);
                bgfx::destroy(filteredCubeMap);
                bgfx::destroy(irradianceMap);
            }
        }

        uint16_t width = 0u;
        uint16_t irradianceMapSize = 64u;

        // PBR IRB Textures and LUT
        bgfx::UniformHandle u_params = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle u_sourceCubeMap = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle sourceCubeMap = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle filteredCubeMap = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle irradianceMap = BGFX_INVALID_HANDLE;
        bgfx::ProgramHandle preFilteringProgram = BGFX_INVALID_HANDLE;
        bgfx::ProgramHandle irradianceProgram = BGFX_INVALID_HANDLE;

        bool rendered = false;
        bool destroyTextureOnClose = true;
    };

    class SceneUniforms
    {
    public:
        bgfx::UniformHandle m_cameraPos = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle m_envParams = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle m_brdfLUT = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle m_prefilteredEnv = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle m_irradiance = BGFX_INVALID_HANDLE;

        void init() {
            m_cameraPos = bgfx::createUniform("u_cameraPos", bgfx::UniformType::Vec4);
            m_envParams = bgfx::createUniform("u_envParams", bgfx::UniformType::Vec4);
            m_brdfLUT = bgfx::createUniform("s_brdfLUT", bgfx::UniformType::Sampler);
            m_prefilteredEnv = bgfx::createUniform("s_prefilteredEnv", bgfx::UniformType::Sampler);
            m_irradiance = bgfx::createUniform("s_irradiance", bgfx::UniformType::Sampler);

        }

        void destroy() {
            bgfx::destroy(m_cameraPos);
            bgfx::destroy(m_brdfLUT);
            bgfx::destroy(m_prefilteredEnv);
            bgfx::destroy(m_irradiance);
        }
    };

    class ExampleIbl : public entry::AppI
    {
    public:
        ExampleIbl(const char* _name, const char* _description) : entry::AppI(_name, _description) {}

        void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
        {
            Args args(_argc, _argv);

            m_width = _width;
            m_height = _height;
            m_debug = BGFX_DEBUG_TEXT;
            m_reset = BGFX_RESET_VSYNC | BGFX_RESET_MAXANISOTROPY | BGFX_RESET_MSAA_X16;

            bgfx::Init initInfo;
            initInfo.type = args.m_type;
            initInfo.vendorId = args.m_pciId;
            initInfo.resolution.width = m_width;
            initInfo.resolution.height = m_height;
            initInfo.resolution.reset = m_reset;
            bgfx::init(initInfo);

            // Enable m_debug text.
            bgfx::setDebug(m_debug);

            // Set view 0 clear state.
            bgfx::setViewClear(0
                , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL
                , 0x000000ff
                , 1.0f
                , 0
            );

            m_caps = bgfx::getCaps();
            m_computeSupported = !!(m_caps->supported & BGFX_CAPS_COMPUTE);

            if (!m_computeSupported) {
                return;
            }

            bae::init(m_skyboxMatType);
            bae::init(m_pbrIrbMatType);
            m_sceneUniforms.init();

            bae::Vertex::init();
            // Set the matType for both opaque and transparent passes
            m_scene.opaqueMatType = m_pbrIrbMatType;
            m_scene.transparentMatType = m_pbrIrbMatType;
            // Lets load all the meshes
            m_scene.load("meshes/FlightHelmet/", "FlightHelmet.gltf");

            m_toneMapParams.width = m_width;
            m_toneMapParams.width = m_height;
            m_toneMapParams.originBottomLeft = m_caps->originBottomLeft;
            m_toneMapPass.init(m_caps);

            m_brdfLutCreator.init();

            m_envMap = loadTexture("textures/pisa_with_mips.ktx");
            m_prefilteredEnvMapCreator.sourceCubeMap = m_envMap;
            m_prefilteredEnvMapCreator.width = 1024u; // Based off size of pisa_with_mips.ktx
            m_prefilteredEnvMapCreator.init();

            // Imgui.
            imguiCreate();

            s_texelHalf = bgfx::RendererType::Direct3D9 == m_caps->rendererType ? 0.5f : 0.0f;

            // Init camera
            cameraCreate();
            cameraSetPosition({ -3.5f, 0.0f, 7.0f });
            cameraSetHorizontalAngle(bx::atan2(3.5f, -7.0f));
            cameraSetVerticalAngle(bx::toRad(-10.0f));

            m_oldWidth = 0;
            m_oldHeight = 0;
            m_oldReset = m_reset;

            m_time = 0.0f;
        }

        virtual int shutdown() override
        {
            if (m_computeSupported) {
                if (bgfx::isValid(m_hdrFrameBuffer))
                {
                    bgfx::destroy(m_hdrFrameBuffer);
                }
                m_toneMapPass.destroy();
                m_prefilteredEnvMapCreator.destroy();
                m_brdfLutCreator.destroy();

                cameraDestroy();
                imguiDestroy();
            }

            // Shutdown bgfx.
            bgfx::shutdown();

            return 0;
        }

        void initializeFrameBuffers() {
            // Recreate variable size render targets when resolution changes.
            m_oldWidth = m_width;
            m_oldHeight = m_height;
            m_oldReset = m_reset;

            uint32_t msaa = (m_reset & BGFX_RESET_MSAA_MASK) >> BGFX_RESET_MSAA_SHIFT;

            if (bgfx::isValid(m_hdrFrameBuffer))
            {
                bgfx::destroy(m_hdrFrameBuffer);
            }


            const uint64_t tsFlags = 0
                | BGFX_SAMPLER_MIN_POINT
                | BGFX_SAMPLER_MAG_POINT
                | BGFX_SAMPLER_MIP_POINT
                | BGFX_SAMPLER_U_CLAMP
                | BGFX_SAMPLER_V_CLAMP
                ;

            m_toneMapParams.width = m_width;
            m_toneMapParams.height = m_height;

            m_hdrFbTextures[0] = bgfx::createTexture2D(
                uint16_t(m_width)
                , uint16_t(m_height)
                , false
                , 1
                , bgfx::TextureFormat::RGBA16F
                , (uint64_t(msaa + 1) << BGFX_TEXTURE_RT_MSAA_SHIFT) | BGFX_SAMPLER_UVW_CLAMP | BGFX_SAMPLER_POINT
            );

            const uint64_t textureFlags = BGFX_TEXTURE_RT_WRITE_ONLY | (uint64_t(msaa + 1) << BGFX_TEXTURE_RT_MSAA_SHIFT);

            bgfx::TextureFormat::Enum depthFormat;
            if (bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, textureFlags)) {
                depthFormat = bgfx::TextureFormat::D24S8;
            }
            else {
                depthFormat = bgfx::TextureFormat::D32;
            }

            m_hdrFbTextures[1] = bgfx::createTexture2D(
                uint16_t(m_width)
                , uint16_t(m_height)
                , false
                , 1
                , depthFormat
                , textureFlags
            );

            bgfx::setName(m_hdrFbTextures[0], "HDR Buffer");

            m_hdrFrameBuffer = bgfx::createFrameBuffer(BX_COUNTOF(m_hdrFbTextures), m_hdrFbTextures, true);

        }

        bool update() override
        {

            if (entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState)) {
                return false;
            }

            if (!m_computeSupported) {
                return false;
            }

            bgfx::ViewId viewId = 0;

            if (!m_brdfLutCreator.rendered) {
                m_brdfLutCreator.renderLUT(viewId);
            }
            // Have to still skip using this viewId, or reset the views to remove all the associated state.
            viewId++;

            if (!m_prefilteredEnvMapCreator.rendered) {
                m_prefilteredEnvMapCreator.render(viewId);
            }
            viewId++;

            if (!bgfx::isValid(m_hdrFrameBuffer)
                || m_oldWidth != m_width
                || m_oldHeight != m_height
                || m_oldReset != m_reset) {
                initializeFrameBuffers();
            }

            imguiBeginFrame(m_mouseState.m_mx
                , m_mouseState.m_my
                , (m_mouseState.m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
                | (m_mouseState.m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
                | (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
                , m_mouseState.m_mz
                , uint16_t(m_width)
                , uint16_t(m_height)
            );

            showExampleDialog(this);

            ImGui::SetNextWindowPos(
                ImVec2(m_width - m_width / 5.0f - 10.0f, 10.0f)
                , ImGuiCond_FirstUseEver
            );
            ImGui::SetNextWindowSize(
                ImVec2(m_width / 5.0f, m_height / 3.0f)
                , ImGuiCond_FirstUseEver
            );
            ImGui::Begin("Settings"
                , NULL
                , 0
            );

            ImGui::End();

            imguiEndFrame();


            bgfx::ViewId meshPass = viewId++;
            bgfx::setViewRect(meshPass, 0, 0, uint16_t(m_width), uint16_t(m_height));
            bgfx::setViewClear(meshPass, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
            bgfx::setViewFrameBuffer(meshPass, m_hdrFrameBuffer);
            bgfx::setViewName(meshPass, "Draw Meshes");

            // Set views.
            bgfx::ViewId skyboxPass = viewId++;
            bgfx::setViewName(skyboxPass, "Skybox");
            bgfx::setViewRect(skyboxPass, 0, 0, bgfx::BackbufferRatio::Equal);
            bgfx::setViewFrameBuffer(skyboxPass, m_hdrFrameBuffer);

            int64_t now = bx::getHPCounter();
            static int64_t last = now;
            const int64_t frameTime = now - last;
            last = now;
            const double freq = double(bx::getHPFrequency());
            const float deltaTime = bx::max(0.0001f, (float)(frameTime / freq));
            m_time += deltaTime;

            float orthoProjection[16];
            bx::mtxOrtho(orthoProjection, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, m_caps->homogeneousDepth);

            float proj[16];
            bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 1000.0f, bgfx::getCaps()->homogeneousDepth);

            // Update camera
            float view[16];
            cameraUpdate(0.5f * deltaTime, m_mouseState);
            cameraGetViewMtx(view);
            bx::Vec3 cameraPos = cameraGetPosition();

            float viewCopy[16] = {};
            bx::memCopy(viewCopy, view, 16 * sizeof(view[0]));
            // Remove translation...
            viewCopy[12] = 0.0f;
            viewCopy[13] = 0.0f;
            viewCopy[14] = 0.0f;

            float rotationViewProj[16] = {};
            bx::mtxMul(rotationViewProj, viewCopy, proj);
            float invRotationViewProj[16] = {};
            bx::mtxInverse(invRotationViewProj, rotationViewProj);

            // Render skybox into view hdrSkybox.
            bgfx::setTexture(0, m_skyboxMatType.getUniformHandle("s_envMap"), m_envMap);
            bgfx::setUniform(m_skyboxMatType.getUniformHandle("u_invRotationViewProj"), invRotationViewProj);
            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LEQUAL);
            bgfx::setViewTransform(skyboxPass, nullptr, orthoProjection);
            bae::setScreenSpaceQuad((float)m_width, (float)m_height, true);
            bgfx::submit(skyboxPass, m_skyboxMatType.program);

            // Set view and projection matrix

            uint64_t stateOpaque = 0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS;

            uint64_t stateTransparent = 0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
                | BGFX_STATE_BLEND_ALPHA;

            bgfx::setViewTransform(meshPass, view, proj);

            // Not moving our models at all
            glm::mat4 mtx = glm::identity<glm::mat4>();
            mtx = glm::rotate(mtx, bx::kPi, glm::vec3(0.0f, 1.0f, 0.0f));

            // Opaque Meshes
            bgfx::ProgramHandle program = m_scene.opaqueMatType.program;
            bae::MaterialType& matType = m_scene.opaqueMatType;
            bgfx::UniformHandle normalTransformHandle = matType.getUniformHandle("u_normalTransform");
            bgfx::UniformHandle diffuseMapHandle = matType.getUniformHandle("s_diffuseMap");
            bgfx::UniformHandle normalMapHandle = matType.getUniformHandle("s_normalMap");
            bgfx::UniformHandle metallicRoughnessMapHandle = matType.getUniformHandle("s_metallicRoughnessMap");

            // Render all our opaque meshes
            for (size_t i = 0; i < m_scene.opaqueMeshes.meshes.size(); ++i) {
                const auto& mesh = m_scene.opaqueMeshes.meshes[i];

                bgfx::setTransform(glm::value_ptr(mtx));
                // Not sure if this should be part of the material?
                bgfx::setUniform(normalTransformHandle, glm::value_ptr(glm::transpose(glm::inverse(mtx))));
                bgfx::setUniform(m_sceneUniforms.m_cameraPos, &cameraPos.x);

                float envParams[] = { bx::log2(float(m_prefilteredEnvMapCreator.width)), 0.0f, 0.0f, 0.0f };
                bgfx::setUniform(m_sceneUniforms.m_envParams, envParams);

                bgfx::setTexture(3, m_sceneUniforms.m_brdfLUT, m_brdfLutCreator.getLUT());
                bgfx::setTexture(4, m_sceneUniforms.m_prefilteredEnv, m_prefilteredEnvMapCreator.getPrefilteredMap());
                bgfx::setTexture(5, m_sceneUniforms.m_irradiance, m_prefilteredEnvMapCreator.getIrradianceMap());

                bgfx::setState(stateOpaque);
                bgfx::setIndexBuffer(mesh.indexHandle);
                bgfx::setVertexBuffer(0, mesh.vertexHandle);
                bae::Material& material = m_scene.opaqueMeshes.materials[i];
                bgfx::setTexture(0, diffuseMapHandle, material.diffuse);
                bgfx::setTexture(1, normalMapHandle, material.normal);
                bgfx::setTexture(2, metallicRoughnessMapHandle, material.metallicRoughness);
                bgfx::submit(meshPass, program);
            }

            // Transparent Meshes
            matType = m_scene.transparentMatType;
            program = m_scene.transparentMatType.program;
            normalTransformHandle = matType.getUniformHandle("u_normalTransform");
            diffuseMapHandle = matType.getUniformHandle("s_diffuseMap");
            normalMapHandle = matType.getUniformHandle("s_normalMap");
            metallicRoughnessMapHandle = matType.getUniformHandle("s_metallicRoughnessMap");

            // Render all our transparent meshes
            for (size_t i = 0; i < m_scene.transparentMeshes.meshes.size(); ++i) {
                bgfx::setTransform(glm::value_ptr(mtx));
                // Not sure if this should be part of the material?
                bgfx::setUniform(normalTransformHandle, glm::value_ptr(glm::transpose(glm::inverse(mtx))));
                bgfx::setUniform(m_sceneUniforms.m_cameraPos, &cameraPos.x);
                bgfx::setTexture(3, m_sceneUniforms.m_brdfLUT, m_brdfLutCreator.getLUT());
                bgfx::setTexture(4, m_sceneUniforms.m_prefilteredEnv, m_prefilteredEnvMapCreator.getPrefilteredMap());
                bgfx::setTexture(5, m_sceneUniforms.m_irradiance, m_prefilteredEnvMapCreator.getIrradianceMap());

                bgfx::setState(stateTransparent);
                const auto& mesh = m_scene.transparentMeshes.meshes[i];
                bgfx::setIndexBuffer(mesh.indexHandle);
                bgfx::setVertexBuffer(0, mesh.vertexHandle);

                bae::Material& material = m_scene.transparentMeshes.materials[i];
                bgfx::setTexture(0, diffuseMapHandle, material.diffuse);
                bgfx::setTexture(1, normalMapHandle, material.normal);
                bgfx::setTexture(2, metallicRoughnessMapHandle, material.metallicRoughness);
                bgfx::submit(meshPass, program);
            }

            m_toneMapPass.render(m_hdrFbTextures[0], m_toneMapParams, deltaTime, viewId);

            bgfx::frame();

            return true;
        }

        entry::MouseState m_mouseState;

        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_debug;
        uint32_t m_reset;

        uint32_t m_oldWidth;
        uint32_t m_oldHeight;
        uint32_t m_oldReset;

        float m_totalBrightness = 1.0f;

        bae::MaterialType m_skyboxMatType = {
            "skybox",
            BGFX_INVALID_HANDLE,
            {
                {"s_envMap", {bgfx::UniformType::Sampler}},
                {"u_invRotationViewProj", {bgfx::UniformType::Mat4}}
            },
        };

        bae::MaterialType m_pbrIrbMatType = {
            "pbr_ibl",
            BGFX_INVALID_HANDLE,
            {
                {"s_diffuseMap", {bgfx::UniformType::Sampler}},
                {"s_normalMap", {bgfx::UniformType::Sampler}},
                {"s_metallicRoughnessMap", {bgfx::UniformType::Sampler}},
                {"u_normalTransform", {bgfx::UniformType::Mat4}},
            },
        };

        // PBR IRB Textures and LUT
        bgfx::TextureHandle m_envMap = BGFX_INVALID_HANDLE;
        // Buffer to put final outputs into
        bgfx::TextureHandle m_hdrFbTextures[2];
        bgfx::FrameBufferHandle m_hdrFrameBuffer = BGFX_INVALID_HANDLE;

        bae::ToneMapParams m_toneMapParams;
        bae::ToneMapping m_toneMapPass;

        BrdfLutCreator m_brdfLutCreator;
        CubeMapFilterer m_prefilteredEnvMapCreator;

        bae::Scene m_scene;
        SceneUniforms m_sceneUniforms;

        const bgfx::Caps* m_caps;
        float m_time;

        bool m_computeSupported = true;
    };

}  // namespace example

ENTRY_IMPLEMENT_MAIN(
    example::ExampleIbl,
    "04-pbr-ibl",
    "Demo of image based lighting with multi-scattering support.");
