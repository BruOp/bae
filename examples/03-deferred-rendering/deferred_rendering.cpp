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
#include "bae/Offscreen.h"
#include "bae/Tonemapping.h"
#include "bae/IcosahedronFactory.h"

namespace example
{
#define SAMPLER_POINT_CLAMP (BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP)

    static float s_texelHalf = 0.0f;


    class LightSet {
    public:
        bae::Mesh volumeMesh;
        std::vector<glm::vec4> lightPosData;
        std::vector<glm::vec4> lightColorIntensityData;

        void init()
        {
            bae::IcosahedronFactory factory{ 2 };
            volumeMesh = factory.getMesh();
        }

        void destroy()
        {
            bgfx::destroy(volumeMesh.vertexHandle);
            bgfx::destroy(volumeMesh.indexHandle);
        }

        size_t addLight(const glm::vec3& color, const float intensity, const glm::vec3& position)
        {
            constexpr float epsilon = 1.0f / 127.0f;
            float radius = bx::sqrt(intensity / epsilon);

            size_t idx = lightPosData.size();
            lightPosData.push_back(glm::vec4{ position, radius });
            lightColorIntensityData.push_back(glm::vec4{ color, intensity });
            return idx;
        }

        size_t size()
        {
            return lightPosData.size();
        }
    };

    class ExampleDeferred : public entry::AppI
    {
    public:
        ExampleDeferred(const char* _name, const char* _description) : entry::AppI(_name, _description) {}

        void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
        {
            Args args(_argc, _argv);

            m_width = _width;
            m_height = _height;
            m_debug = BGFX_DEBUG_TEXT;
            m_reset = BGFX_RESET_VSYNC;

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
            bae::Vertex::init();

            bae::init(m_deferredPbrMatType);
            bae::init(m_lightStencilMatType);
            bae::init(m_pointLightVolumeMatType);
            bae::init(m_passthroughMatType);

            m_scene.opaqueMatType = m_deferredPbrMatType;
            //m_scene.transparentMatType = pbrMatType;
            // Lets load all the meshes
            m_scene.load("meshes/pbr_sponza/", "sponza.gltf");
            bae::init(m_sceneUniforms);

            std::vector<glm::vec3> colors = {
                { 1.0f, 1.0f, 1.0f },
                { 1.0f, 0.1f, 0.1f },
                { 0.1f, 1.0f, 0.1f },
                { 0.1f, 0.1f, 1.0f },
                { 1.0f, 0.1f, 1.0f },
                { 1.0f, 1.0f, 0.1f },
                { 0.1f, 1.0f, 1.0f },
            };

            constexpr float totalBrightness = 10.0f;

            bae::BasicVertex::init();
            m_lightSet.init();
            int NUM_LIGHTS = 8;
            for (size_t i = 0; i < NUM_LIGHTS; i++) {
                m_lightSet.addLight(
                    colors[i % colors.size()],
                    totalBrightness / float(NUM_LIGHTS),
                    { 10.0f * i, 5.0f, 0.0f }
                );
            }

            bae::ScreenSpaceQuadVertex::init();

            m_toneMapParams.width = m_width;
            m_toneMapParams.width = m_height;
            m_toneMapParams.originBottomLeft = m_caps->originBottomLeft;
            m_toneMapPass.init(m_caps);


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

                if (bgfx::isValid(m_gBuffer))
                {
                    bgfx::destroy(m_gBuffer);
                }

                m_toneMapPass.destroy();

                m_lightSet.destroy();
                // Cleanup.
                destroy(m_sceneUniforms);
                destroy(m_scene);
                destroy(m_passthroughMatType);
                destroy(m_lightStencilMatType);
                destroy(m_pointLightVolumeMatType);
                destroy(m_deferredPbrMatType);

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


            if (bgfx::isValid(m_gBuffer))
            {
                bgfx::destroy(m_gBuffer);
                bgfx::destroy(m_lightGBuffer);
                m_gbufferTex[0].idx = bgfx::kInvalidHandle;
                m_gbufferTex[1].idx = bgfx::kInvalidHandle;
                m_gbufferTex[2].idx = bgfx::kInvalidHandle;
                m_gbufferTex[3].idx = bgfx::kInvalidHandle;
                m_gbufferTex[4].idx = bgfx::kInvalidHandle;
            }

            const uint64_t tsFlags = 0
                | BGFX_SAMPLER_MIN_POINT
                | BGFX_SAMPLER_MAG_POINT
                | BGFX_SAMPLER_MIP_POINT
                | BGFX_SAMPLER_U_CLAMP
                | BGFX_SAMPLER_V_CLAMP
                ;

            bgfx::Attachment gbufferAt[5];

            m_gbufferTex[0] = bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT | tsFlags);
            m_gbufferTex[1] = bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::RGBA16F, BGFX_TEXTURE_RT | tsFlags);
            m_gbufferTex[2] = bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::R32F, BGFX_TEXTURE_RT | tsFlags);
            m_gbufferTex[3] = bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::RGBA16F, BGFX_TEXTURE_RT | tsFlags);

            gbufferAt[0].init(m_gbufferTex[0]);
            gbufferAt[1].init(m_gbufferTex[1]);
            gbufferAt[2].init(m_gbufferTex[2]);
            gbufferAt[3].init(m_gbufferTex[3]);

            m_gbufferTex[4] = bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT | tsFlags);
            gbufferAt[4].init(m_gbufferTex[4]);

            m_gBuffer = bgfx::createFrameBuffer(BX_COUNTOF(gbufferAt), gbufferAt, true);
            m_lightGBuffer = bgfx::createFrameBuffer(2, &gbufferAt[3], false);

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

            imguiEndFrame();

            bgfx::ViewId meshPass = 0;
            bgfx::setViewRect(meshPass, 0, 0, uint16_t(m_width), uint16_t(m_height));
            bgfx::setViewFrameBuffer(meshPass, m_gBuffer);
            bgfx::setViewName(meshPass, "Draw Meshes");

            bgfx::ViewId lightVolumePass = 1;
            bgfx::setViewFrameBuffer(lightVolumePass, m_lightGBuffer);
            bgfx::setViewRect(lightVolumePass, 0, 0, uint16_t(m_width), uint16_t(m_height));
            bgfx::setViewMode(lightVolumePass, bgfx::ViewMode::Sequential);
            bgfx::setViewName(lightVolumePass, "Light Volume Pass");
/*
            bgfx::ViewId passthroughPass = 2;
            bgfx::setViewRect(passthroughPass, 0, 0, uint16_t(m_width), uint16_t(m_height));
            bgfx::setViewName(passthroughPass, "Pass Through");
*/
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

            uint64_t stateOpaque = 0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW;

            // Set view 0 default viewport.
            bx::Vec3 cameraPos = cameraGetPosition();
            bgfx::setUniform(m_sceneUniforms.cameraPos, &cameraPos.x);

            // Render all our opaque meshes
            bgfx::UniformHandle normalTransformHandle = m_scene.opaqueMatType.uniformInfo["normalTransform"].handle;
            bgfx::ProgramHandle program = m_scene.opaqueMatType.program;
            for (size_t i = 0; i < m_scene.opaqueMeshes.meshes.size(); ++i) {
                const auto& mesh = m_scene.opaqueMeshes.meshes[i];

                bgfx::setTransform(glm::value_ptr(mtx));
                // Not sure if this should be part of the material?
                bgfx::setUniform(normalTransformHandle, glm::value_ptr(glm::transpose(glm::inverse(mtx))));

                bgfx::setState(stateOpaque);
                bgfx::setIndexBuffer(mesh.indexHandle);
                bgfx::setVertexBuffer(0, mesh.vertexHandle);
                setUniforms(m_scene.opaqueMeshes.materials[i], m_scene.opaqueMatType);
                bgfx::submit(meshPass, program);
            }

            // Update our light positions so they fly around the atrium
            {
                // Totally ad hoc variables for the size of the atrium model
                constexpr float atriumLength = 55.0f;
                constexpr float atriumWidth = 24.0f;
                constexpr float atriumHeight = 45.0f;
                constexpr float velocity = 0.1f;

                float N = float(m_lightSet.size());
                for (size_t i = 0; i < N; ++i) {
                    float radiusCoeff = (float(i % 8) + 1.0f) / 8.0f;
                    // These two are just the equations for an ellipse, with the radius scaled by radiusCoeff and the speed scaled by velocity
                    m_lightSet.lightPosData[i].x = radiusCoeff * atriumLength * bx::cos(velocity * m_time / radiusCoeff + bx::kPi2 * i / 26.0f);
                    m_lightSet.lightPosData[i].z = radiusCoeff * atriumWidth * bx::sin(velocity * m_time / radiusCoeff + bx::kPi2 * i / 26.0f);
                    m_lightSet.lightPosData[i].y = atriumHeight * (i + 1) / N;
                }
            }

            bgfx::setViewTransform(lightVolumePass, view, proj);
            // Lets render our light volumes
            for (size_t i = 0; i < m_lightSet.size(); ++i) {
                // We need to set up the stencil state
                uint64_t stencilState = 0
                    | BGFX_STATE_DEPTH_TEST_LESS;

                uint32_t frontStencilFunc = BGFX_STENCIL_TEST_ALWAYS
                    | BGFX_STENCIL_FUNC_REF(0)
                    | BGFX_STENCIL_FUNC_RMASK(0xFF)
                    | BGFX_STENCIL_OP_FAIL_S_KEEP
                    | BGFX_STENCIL_OP_FAIL_Z_INCR
                    | BGFX_STENCIL_OP_PASS_Z_KEEP;
                uint32_t backStencilFunc = BGFX_STENCIL_TEST_ALWAYS
                    | BGFX_STENCIL_FUNC_REF(0)
                    | BGFX_STENCIL_FUNC_RMASK(0xFF)
                    | BGFX_STENCIL_OP_FAIL_S_KEEP
                    | BGFX_STENCIL_OP_FAIL_Z_KEEP
                    | BGFX_STENCIL_OP_PASS_Z_INCR;

                glm::mat4 modelTransform = glm::identity<glm::mat4>();
                glm::vec3 position { m_lightSet.lightPosData[i].x, m_lightSet.lightPosData[i].y, m_lightSet.lightPosData[i].z };
                glm::vec3 scale { m_lightSet.lightPosData[i].w };
                modelTransform = glm::scale(
                    glm::translate(modelTransform, position),
                    scale
                );

                bgfx::setTransform(glm::value_ptr(modelTransform));
                bgfx::setState(stencilState);
                bgfx::setStencil(frontStencilFunc, backStencilFunc);
                bgfx::setIndexBuffer(m_lightSet.volumeMesh.indexHandle);
                bgfx::setVertexBuffer(0, m_lightSet.volumeMesh.vertexHandle);
                bgfx::submit(lightVolumePass, m_lightStencilMatType.program);

                uint64_t lightVolumeState = 0
                    | BGFX_STATE_WRITE_RGB
                    | BGFX_STATE_CULL_CW
                    | BGFX_STATE_BLEND_ADD;
                frontStencilFunc = BGFX_STENCIL_TEST_EQUAL
                    | BGFX_STENCIL_FUNC_RMASK(0xFF)
                    | BGFX_STENCIL_FUNC_REF(0);
                backStencilFunc = BGFX_STENCIL_TEST_EQUAL
                    | BGFX_STENCIL_FUNC_RMASK(0xFF)
                    | BGFX_STENCIL_FUNC_REF(0)
                    | BGFX_STENCIL_OP_FAIL_S_REPLACE;
                bgfx::setTransform(glm::value_ptr(modelTransform));
                bgfx::setIndexBuffer(m_lightSet.volumeMesh.indexHandle);
                bgfx::setVertexBuffer(0, m_lightSet.volumeMesh.vertexHandle);
                bgfx::setState(lightVolumeState);
                bgfx::setStencil(frontStencilFunc, backStencilFunc);

                // Set uniforms
                // framebuffer textures
                // light properties

                bgfx::setTexture(0, m_pointLightVolumeMatType.getUniformHandle("diffuseRT"), m_gbufferTex[0], BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP);
                bgfx::setTexture(1, m_pointLightVolumeMatType.getUniformHandle("normalRT"), m_gbufferTex[1], BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP);
                bgfx::setTexture(2, m_pointLightVolumeMatType.getUniformHandle("depthRT"), m_gbufferTex[2], BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP);
                bgfx::setUniform(m_pointLightVolumeMatType.getUniformHandle("lightPosRadius"), glm::value_ptr(m_lightSet.lightPosData[i]));
                bgfx::setUniform(m_pointLightVolumeMatType.getUniformHandle("lightColorIntensity"), glm::value_ptr(m_lightSet.lightColorIntensityData[i]));
                bgfx::submit(lightVolumePass, m_pointLightVolumeMatType.program);
            }
/*
            float orthoProjection[16];
            bx::mtxOrtho(orthoProjection, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, m_caps->homogeneousDepth);
            bgfx::setViewTransform(passthroughPass, nullptr, orthoProjection);
*/
            /*bgfx::setTexture(0, m_passthroughMatType.getUniformHandle("s_diffuseRT"), m_gbufferTex[3], BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP);
            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
            bae::setScreenSpaceQuad(float(m_width), float(m_height), m_caps->originBottomLeft);
            bgfx::submit(passthroughPass, m_passthroughMatType.program);
*/

            //uint64_t stateTransparent = 0
            //    | BGFX_STATE_WRITE_RGB
            //    | BGFX_STATE_WRITE_A
            //    | BGFX_STATE_DEPTH_TEST_LESS
            //    | BGFX_STATE_CULL_CCW
            //    | BGFX_STATE_MSAA
            //    | BGFX_STATE_BLEND_ALPHA;


            //// Render all our transparent meshes
            //program = m_scene.transparentMatType.program;
            //for (size_t i = 0; i < m_scene.transparentMeshes.meshes.size(); ++i) {
            //    bgfx::setTransform(glm::value_ptr(mtx));
            //    // Not sure if this should be part of the material?
            //    bgfx::setUniform(normalTransformHandle, glm::value_ptr(glm::transpose(glm::inverse(mtx))));

            //    bgfx::setState(stateTransparent);
            //    const auto& mesh = m_scene.transparentMeshes.meshes[i];
            //    bgfx::setIndexBuffer(mesh.indexHandle);
            //    bgfx::setVertexBuffer(0, mesh.vertexHandle);
            //    setUniforms(m_scene.transparentMeshes.materials[i], m_scene.transparentMatType);

            //    bgfx::submit(meshPass, program);
            //}

            m_toneMapPass.render(m_gbufferTex[3], m_toneMapParams, deltaTime);

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

        bae::MaterialType m_deferredPbrMatType = {
            "deferred_pbr",
            BGFX_INVALID_HANDLE,
            {
                {"diffuseMap", {bgfx::UniformType::Sampler}},
                {"normalMap", {bgfx::UniformType::Sampler}},
                {"metallicRoughnessMap", {bgfx::UniformType::Sampler}},
                {"normalTransform", {bgfx::UniformType::Mat4}},
            },
        };

        bae::MaterialType m_lightStencilMatType = {
            "light_stencil",
            BGFX_INVALID_HANDLE,
            {},
        };

        bae::MaterialType m_pointLightVolumeMatType = {
            "point_light_volume",
            BGFX_INVALID_HANDLE,
            {
                {"diffuseRT", {bgfx::UniformType::Sampler}},
                {"normalRT", {bgfx::UniformType::Sampler}},
                {"depthRT", {bgfx::UniformType::Sampler}},
                {"lightColorIntensity", {bgfx::UniformType::Vec4}},
                {"lightPosRadius", {bgfx::UniformType::Vec4}},
            },
        };

        bae::MaterialType m_passthroughMatType = {
            "passthrough",
            BGFX_INVALID_HANDLE,
            {
                {"s_diffuseRT", {bgfx::UniformType::Sampler}},
            },
        };

        bae::Scene m_scene;
        bae::SceneUniforms m_sceneUniforms;
        LightSet m_lightSet;

        // Deferred passes
        bgfx::FrameBufferHandle m_gBuffer = BGFX_INVALID_HANDLE;
        bgfx::FrameBufferHandle m_lightGBuffer = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle m_gbufferTex[5];

        // Buffer to put final outputs into
        bgfx::TextureHandle m_hdrFbTextures[2];
        bgfx::FrameBufferHandle m_hdrFrameBuffer = BGFX_INVALID_HANDLE;

        bae::ToneMapParams m_toneMapParams;
        bae::ToneMapping m_toneMapPass;

        bool m_computeSupported = true;

        const bgfx::Caps* m_caps;
        float m_time;
    };

}  // namespace example

ENTRY_IMPLEMENT_MAIN(
    example::ExampleDeferred,
    "03-deferred-rendering",
    "Rendering sponza using deferred rendering.");
