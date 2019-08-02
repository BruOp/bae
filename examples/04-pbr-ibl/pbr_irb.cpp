/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <array>
#include "bgfx_utils.h"
#include "common.h"
#include "imgui/imgui.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "bae/Tonemapping.h"
#include "bae/IcosahedronFactory.h"

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
            const std::string shader = "cs_prefilter_env_map";
            program = loadProgram(shader.c_str(), nullptr);
            uint64_t flags = BGFX_TEXTURE_COMPUTE_WRITE;
            u_sourceCubeMap = bgfx::createUniform("u_source", bgfx::UniformType::Sampler);
            u_params = bgfx::createUniform("u_params", bgfx::UniformType::Vec4);
            filteredCubeMap = bgfx::createTextureCube(width, true, 1, bgfx::TextureFormat::RGBA16F, flags);
            bgfx::setName(filteredCubeMap, "Prefilter Env Map");
        }

        bgfx::TextureHandle getCubeMap()
        {
            return filteredCubeMap;
        }

        void render(bgfx::ViewId view)
        {
            const uint16_t threadCount = 8u;
            bgfx::setViewName(view, "Prefilter Env Map pass");

            float maxMipLevel = bx::log2(float(width));
            for (float mipLevel = 0; mipLevel <= maxMipLevel; ++mipLevel)
            {
                uint16_t mipWidth = width / uint16_t(bx::pow(2.0f, mipLevel));
                float roughness = mipLevel / maxMipLevel;
                float params[] = { roughness, float(mipLevel), float(width), 0.0f };
                bgfx::setUniform(u_params, params);
                bgfx::setTexture(0, u_sourceCubeMap, sourceCubeMap);
                bgfx::setImage(1, filteredCubeMap, uint8_t(mipLevel), bgfx::Access::Write, bgfx::TextureFormat::RGBA16F);
                bgfx::dispatch(view, program, mipWidth / threadCount, mipWidth / threadCount, 1);
            }
            rendered = true;
        }

        void destroy()
        {
            bgfx::destroy(program);
            bgfx::destroy(u_sourceCubeMap);
            bgfx::destroy(u_params);

            if (destroyTextureOnClose) {
                bgfx::destroy(sourceCubeMap);
                bgfx::destroy(filteredCubeMap);
            }
        }

        uint16_t width = 0u;

        // PBR IRB Textures and LUT
        bgfx::UniformHandle u_params;
        bgfx::UniformHandle u_sourceCubeMap;
        bgfx::TextureHandle sourceCubeMap;
        bgfx::TextureHandle filteredCubeMap;
        bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

        bool rendered = false;
        bool destroyTextureOnClose = true;
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
            m_reset = BGFX_RESET_VSYNC | BGFX_RESET_MAXANISOTROPY;

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

            // Create vertex stream declaration.
            bae::BasicVertex::init();

            m_toneMapParams.width = m_width;
            m_toneMapParams.width = m_height;
            m_toneMapParams.originBottomLeft = m_caps->originBottomLeft;
            m_toneMapPass.init(m_caps);

            m_brdfPass.init();
            m_brdfLUT = m_brdfPass.getLUT();

            m_prefilteredEnvMapCreator.sourceCubeMap = loadTexture("textures/pisa.ktx");
            m_prefilteredEnvMapCreator.width = 1024u; // Based off size of pisa.ktx
            m_prefilteredEnvMapCreator.init();

            // Imgui.
            imguiCreate();

            s_texelHalf = bgfx::RendererType::Direct3D9 == m_caps->rendererType ? 0.5f : 0.0f;

            // Init camera
            cameraCreate();
            cameraSetPosition({ 0.0f, 10.5f, 0.0f });

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
                m_brdfPass.destroy();
                // Cleanup.
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
            bgfx::ViewId viewId = 0;

            if (entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState)) {
                return false;
            }

            if (!m_computeSupported) {
                return false;
            }

            if (!m_brdfPass.rendered) {
                m_brdfPass.renderLUT(viewId++);
            }

            if (!m_prefilteredEnvMapCreator.rendered) {
                m_prefilteredEnvMapCreator.render(viewId++);
            }

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

            bgfx::ViewId meshPass = viewId;
            bgfx::setViewRect(meshPass, 0, 0, uint16_t(m_width), uint16_t(m_height));
            bgfx::setViewFrameBuffer(meshPass, m_hdrFrameBuffer);
            bgfx::setViewName(meshPass, "Draw Meshes");

            // This dummy draw call is here to make sure that view 0 is cleared
            // if no other draw calls are submitted to view 0.
            bgfx::touch(meshPass);

            int64_t now = bx::getHPCounter();
            static int64_t last = now;
            const int64_t frameTime = now - last;
            last = now;
            const double freq = double(bx::getHPFrequency());
            const float deltaTime = (float)(frameTime / freq);
            m_time += deltaTime;

            float proj[16];
            bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 1000.0f, bgfx::getCaps()->homogeneousDepth);

            // Update camera
            float view[16];

            cameraUpdate(0.5f * deltaTime, m_mouseState);
            cameraGetViewMtx(view);
            // Set view and projection matrix
            bgfx::setViewTransform(meshPass, view, proj);

            glm::mat4 mtx = glm::identity<glm::mat4>();

            // Set view 0 default viewport.
            bx::Vec3 cameraPos = cameraGetPosition();

            //m_toneMapPass.render(m_hdrFbTextures[0], m_toneMapParams, deltaTime, meshPass + 1);

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

        // PBR IRB Textures and LUT
        bgfx::TextureHandle m_brdfLUT;
        // Buffer to put final outputs into
        bgfx::TextureHandle m_hdrFbTextures[2];
        bgfx::FrameBufferHandle m_hdrFrameBuffer = BGFX_INVALID_HANDLE;

        bae::ToneMapParams m_toneMapParams;
        bae::ToneMapping m_toneMapPass;

        BrdfLutCreator m_brdfPass;
        CubeMapFilterer m_prefilteredEnvMapCreator;

        const bgfx::Caps* m_caps;
        float m_time;

        bool m_computeSupported = true;
    };

}  // namespace example

ENTRY_IMPLEMENT_MAIN(
    example::ExampleIbl,
    "04-pbr-ibl",
    "Demo of image based lighting with multi-scattering support.");
