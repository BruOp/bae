/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <iostream>
#include <array>
#include <bx/rng.h>
#include "bgfx_utils.h"
#include "common.h"
#include "imgui/imgui.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "bae/PhysicallyBasedScene.h"
#include "bae/Tonemapping.h"

namespace example
{
#define SAMPLER_POINT_CLAMP (BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP)

static float s_texelHalf = 0.0f;

class ExampleDeferred : public entry::AppI
{
public:
    ExampleDeferred(const char *_name, const char *_description) : entry::AppI(_name, _description) {}

    void init(int32_t _argc, const char *const *_argv, uint32_t _width, uint32_t _height) override
    {
        Args args(_argc, _argv);

        m_width = _width;
        m_height = _height;
        m_debug = BGFX_DEBUG_TEXT;
        m_reset = BGFX_RESET_MSAA_X8 | BGFX_RESET_VSYNC;

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
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

        const bgfx::Caps *caps = bgfx::getCaps();
        m_computeSupported = !!(caps->supported & BGFX_CAPS_COMPUTE);

        if (!m_computeSupported)
        {
            return;
        }

        // Create vertex stream declaration.
        bae::Vertex::init();

        bae::init(bae::Material::matType);

        // Lets load all the meshes
        m_scene.load("meshes/pbr_sponza/", "sponza.gltf");
        bae::init(m_sceneUniforms);

        std::vector<glm::vec3> colors = {
            {1.0f, 1.0f, 1.0f},
            {1.0f, 0.1f, 0.1f},
            {0.1f, 1.0f, 0.1f},
            {0.1f, 0.1f, 1.0f},
            {1.0f, 0.1f, 1.0f},
            {1.0f, 1.0f, 0.1f},
            {0.1f, 1.0f, 1.0f},
        };

        m_lightSet.init("pointLight");
        int N = m_lightSet.maxLightCount / 2;
        for (size_t i = 0; i < N; i++)
        {
            m_lightSet.addLight(
                colors[i % colors.size()],
                1000.0f / N,
                {10.0f * i, 5.0f, 0.0f});
        }

        m_caps = bgfx::getCaps();
        m_toneMapParams.width = m_width;
        m_toneMapParams.width = m_height;
        m_toneMapParams.originBottomLeft = m_caps->originBottomLeft;
        m_toneMapPass.init();

        // Imgui.
        imguiCreate();

        s_texelHalf = bgfx::RendererType::Direct3D9 == m_caps->rendererType ? 0.5f : 0.0f;

        // Init camera
        cameraCreate();
        cameraSetPosition({0.0f, 10.5f, 0.0f});

        m_oldWidth = 0;
        m_oldHeight = 0;
        m_oldReset = m_reset;

        m_time = 0.0f;
    }

    virtual int shutdown() override
    {
        if (!m_computeSupported)
        {
            return 0;
        }

        m_toneMapPass.destroy();

        // Cleanup.
        m_lightSet.destroy();
        destroy(m_sceneUniforms);
        destroy(m_scene);
        destroy(bae::Material::matType);

        cameraDestroy();

        imguiDestroy();
        // Shutdown bgfx.
        bgfx::shutdown();

        return 0;
    }

    bool update() override
    {
        if (entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState))
        {
            return false;
        }

        if (!m_computeSupported)
        {
            return false;
        }

        if (m_oldWidth != m_width || m_oldHeight != m_height || m_oldReset != m_reset)
        {
            // Recreate variable size render targets when resolution changes.
            m_oldWidth = m_width;
            m_oldHeight = m_height;
            m_oldReset = m_reset;

            uint32_t msaa = (m_reset & BGFX_RESET_MSAA_MASK) >> BGFX_RESET_MSAA_SHIFT;

            if (bgfx::isValid(m_pbrFramebuffer))
            {
                bgfx::destroy(m_pbrFramebuffer);
            }

            m_toneMapParams.width = m_width;
            m_toneMapParams.height = m_height;

            m_pbrFbTextures[0] = bgfx::createTexture2D(
                uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::RGBA16F, (uint64_t(msaa + 1) << BGFX_TEXTURE_RT_MSAA_SHIFT) | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);

            const uint64_t textureFlags = BGFX_TEXTURE_RT_WRITE_ONLY | (uint64_t(msaa + 1) << BGFX_TEXTURE_RT_MSAA_SHIFT);

            bgfx::TextureFormat::Enum depthFormat =
                bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D16, textureFlags) ? bgfx::TextureFormat::D16
                                                                                          : bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, textureFlags) ? bgfx::TextureFormat::D24S8
                                                                                                                                                                        : bgfx::TextureFormat::D32;

            m_pbrFbTextures[1] = bgfx::createTexture2D(
                uint16_t(m_width), uint16_t(m_height), false, 1, depthFormat, textureFlags);

            m_pbrFramebuffer = bgfx::createFrameBuffer(BX_COUNTOF(m_pbrFbTextures), m_pbrFbTextures, true);
        }

        imguiBeginFrame(m_mouseState.m_mx, m_mouseState.m_my, (m_mouseState.m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0) | (m_mouseState.m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0) | (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0), m_mouseState.m_mz, uint16_t(m_width), uint16_t(m_height));

        showExampleDialog(this);

        imguiEndFrame();

        // This dummy draw call is here to make sure that view 0 is cleared
        // if no other draw calls are submitted to view 0.
        bgfx::touch(0);

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
        bgfx::setViewTransform(0, view, proj);

        glm::mat4 mtx = glm::identity<glm::mat4>();
        //bx::mtxRotateXY(glm::value_ptr(mtx), 0.0f, 0.2f * m_time);

        uint64_t stateOpaque = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;

        uint64_t stateTransparent = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_BLEND_ALPHA;

        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
        bx::Vec3 cameraPos = cameraGetPosition();
        bgfx::setUniform(m_sceneUniforms.cameraPos, &cameraPos.x);

        // Update our light positions so they fly around the atrium
        {
            float a = 100.0f;
            float b = 35.0f;
            float N = float(m_lightSet.lightCount);
            for (size_t i = 0; i < m_lightSet.lightCount; ++i)
            {
                float coeff = (float(i % 8) + 1.0f) / 8.0f;
                m_lightSet.lightPosData[i].x = coeff * a * bx::cos(0.1f * m_time / coeff + bx::kPi2 * i / N);
                m_lightSet.lightPosData[i].z = coeff * b * bx::sin(0.1f * m_time / coeff + bx::kPi2 * i / N);
                m_lightSet.lightPosData[i].y = 60.0f * (i + 1) / N;
            }
        }
        m_lightSet.setUniforms();

        bgfx::UniformHandle normalTransformHandle = bae::Material::matType.uniformInfo["normalTransform"].handle;

        // Only using one material type for now, otherwise we'd have to navigate through the meshes' material
        bgfx::ProgramHandle program = bae::Material::matType.program;
        // Render all our meshes
        for (auto &mesh : m_scene.meshes)
        {
            bgfx::setTransform(glm::value_ptr(mtx));
            // Not sure if this should be part of the material?
            bgfx::setUniform(normalTransformHandle, glm::value_ptr(glm::transpose(glm::inverse(mtx))));

            bgfx::setIndexBuffer(mesh.indexHandle);
            bgfx::setVertexBuffer(0, mesh.vertexHandle);
            setUniforms(mesh.material);

            if (mesh.material.hasAlpha)
            {
                bgfx::setState(stateTransparent);
            }
            else
            {
                bgfx::setState(stateOpaque);
            }

            bgfx::submit(0, program);
        }

        m_toneMapPass.render(m_pbrFbTextures[0], m_toneMapParams, deltaTime);

        bgfx::frame();

        return true;
    }

    entry::MouseState m_mouseState;

    bx::RngMwc m_rng;

    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_debug;
    uint32_t m_reset;

    uint32_t m_oldWidth;
    uint32_t m_oldHeight;
    uint32_t m_oldReset;

    bae::Scene m_scene;
    bae::SceneUniforms m_sceneUniforms;
    bae::LightSet m_lightSet;

    bgfx::TextureHandle m_pbrFbTextures[2];
    bgfx::FrameBufferHandle m_pbrFramebuffer = BGFX_INVALID_HANDLE;

    bae::ToneMapParams m_toneMapParams;
    bae::ToneMapping m_toneMapPass;

    bool m_computeSupported = true;

    const bgfx::Caps *m_caps;
    float m_time;
};

} // namespace example

ENTRY_IMPLEMENT_MAIN(
    example::ExampleDeferred,
    "03-deferred-rendering",
    "Rendering sponza using single lighting pass forward.");
