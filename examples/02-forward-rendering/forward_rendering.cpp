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

    static std::vector<glm::vec3> LIGHT_COLORS = {
            { 1.0f, 1.0f, 1.0f },
            { 1.0f, 0.1f, 0.1f },
            { 0.1f, 1.0f, 0.1f },
            { 0.1f, 0.1f, 1.0f },
            { 1.0f, 0.1f, 1.0f },
            { 1.0f, 1.0f, 0.1f },
            { 0.1f, 1.0f, 1.0f },
    };

    class LightSet {
    public:
        uint16_t lightCount = 0;
        uint16_t maxLightCount = 255;  // Has to match whatever we have set in the shader...

        std::vector<glm::vec4> lightPosData;
        std::vector<glm::vec4> lightColorIntensityData;

        // params is bacasically just used to store params.x = lightCount
        bgfx::UniformHandle params = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle lightPos = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle lightColorIntensity = BGFX_INVALID_HANDLE;

        void init(const std::string& lightName)
        {
            auto uniformName = lightName + "_params";
            params = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4);
            uniformName = lightName + "_pos";
            lightPos = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4, maxLightCount);
            uniformName = lightName + "_colorIntensity";
            lightColorIntensity = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4, maxLightCount);

            lightPosData.resize(maxLightCount);
            lightColorIntensityData.resize(maxLightCount);
        }

        void setUniforms() const
        {
            uint32_t paramsArr[4]{ uint32_t(lightCount), 0, 0, 0 };
            bgfx::setUniform(params, paramsArr);
            bgfx::setUniform(lightPos, lightPosData.data(), maxLightCount);
            bgfx::setUniform(lightColorIntensity, lightColorIntensityData.data(), maxLightCount);
        };

        void destroy()
        {
            bgfx::destroy(params);
            bgfx::destroy(lightPos);
            bgfx::destroy(lightColorIntensity);
        }
    };

    bae::MaterialType pbrMatType{
            "textured_physical",
            BGFX_INVALID_HANDLE,
            {
                {"diffuseMap", {bgfx::UniformType::Sampler}},
                {"normalMap", {bgfx::UniformType::Sampler}},
                {"metallicRoughnessMap", {bgfx::UniformType::Sampler}},
                {"normalTransform", {bgfx::UniformType::Mat4}},
            },
    };


    class ExampleForward : public entry::AppI
    {
    public:
        ExampleForward(const char* _name, const char* _description) : entry::AppI(_name, _description) {}

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
                , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
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

            bae::init(pbrMatType);

            // Set the matType for both opaque and transparent passes
            m_scene.opaqueMatType = pbrMatType;
            m_scene.transparentMatType = pbrMatType;
            // Lets load all the meshes
            m_scene.load("meshes/pbr_sponza/", "sponza.gltf");
            bae::init(m_sceneUniforms);


            m_lightSet.init("pointLight");
            m_totalBrightness = 500.0f;

            size_t numColors = LIGHT_COLORS.size();
            m_lightSet.lightCount = 8;

            for (size_t i = 0; i < m_lightSet.maxLightCount; i++) {
                m_lightSet.lightColorIntensityData[i] = glm::vec4{ LIGHT_COLORS[i % numColors], m_totalBrightness / 500.0f };
            }

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
            if (!m_computeSupported) {
                return 0;
            }

            if (bgfx::isValid(m_pbrFramebuffer))
            {
                bgfx::destroy(m_pbrFramebuffer);
            }

            m_toneMapPass.destroy();

            // Cleanup.
            m_lightSet.destroy();
            destroy(m_sceneUniforms);
            destroy(m_scene);
            destroy(pbrMatType);

            cameraDestroy();

            imguiDestroy();
            // Shutdown bgfx.
            bgfx::shutdown();

            return 0;
        }

        bool update() override
        {
            if (entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState)) {
                return false;
            }

            if (!m_computeSupported) {
                return false;
            }

            if (!bgfx::isValid(m_pbrFramebuffer)
                || m_oldWidth != m_width
                || m_oldHeight != m_height
                || m_oldReset != m_reset) {

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

                m_pbrFbTextures[1] = bgfx::createTexture2D(
                    uint16_t(m_width)
                    , uint16_t(m_height)
                    , false
                    , 1
                    , depthFormat
                    , textureFlags
                );

                bgfx::setName(m_pbrFbTextures[0], "HDR Buffer");

                m_pbrFramebuffer = bgfx::createFrameBuffer(BX_COUNTOF(m_pbrFbTextures), m_pbrFbTextures, true);
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

            int lightCount = m_lightSet.lightCount;
            ImGui::SliderInt("Num lights", &lightCount, 1, m_lightSet.maxLightCount);
            ImGui::DragFloat("Total Brightness", &m_totalBrightness, 0.5f, 0.0f, 1000.0f);

            ImGui::End();

            imguiEndFrame();

            bgfx::ViewId meshPass = 0;
            bgfx::setViewFrameBuffer(meshPass, m_pbrFramebuffer);
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

            // Not scaling or translating our scene
            glm::mat4 mtx = glm::identity<glm::mat4>();

            uint64_t stateOpaque = 0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA;

            uint64_t stateTransparent = 0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
                | BGFX_STATE_BLEND_ALPHA;

            // Set view 0 default viewport.
            bgfx::setViewRect(meshPass, 0, 0, uint16_t(m_width), uint16_t(m_height));
            bx::Vec3 cameraPos = cameraGetPosition();
            bgfx::setUniform(m_sceneUniforms.cameraPos, &cameraPos.x);

            {
                m_lightSet.lightCount = uint16_t(lightCount);
                //Move our lights around in a cylinder the size of our scene
                constexpr float sceneWidth = 52.0f;
                constexpr float sceneLength = 24.0f;
                constexpr float sceneHeight = 45.0f;

                float N = float(m_lightSet.lightCount);
                float intensity = m_totalBrightness / N;
                constexpr float EPSILON = 0.01f;
                float radius = bx::sqrt(intensity / EPSILON);

                for (size_t i = 0; i < m_lightSet.maxLightCount; ++i) {
                    // This is pretty ad-hoc...
                    float coeff = (float(i % 8) + 1.0f) / 8.0f;
                    float phaseOffset = bx::kPi2 * i / N;
                    m_lightSet.lightPosData[i].x = coeff * sceneWidth * bx::cos(0.1f * m_time / coeff + phaseOffset);
                    m_lightSet.lightPosData[i].z = coeff * sceneHeight * bx::sin(0.1f * m_time / coeff + phaseOffset);
                    m_lightSet.lightPosData[i].y = sceneHeight * (i + 1) / N;

                    // Set intensity
                    m_lightSet.lightColorIntensityData[i].w = intensity;

                    // Set radius
                    m_lightSet.lightPosData[i].w = radius;
                }
            }
            m_lightSet.setUniforms();

            bgfx::UniformHandle normalTransformHandle = pbrMatType.uniformInfo["normalTransform"].handle;
            bgfx::ProgramHandle program = m_scene.opaqueMatType.program;
            // Render all our opaque meshes
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

            // Render all our transparent meshes
            program = m_scene.transparentMatType.program;
            for (size_t i = 0; i < m_scene.transparentMeshes.meshes.size(); ++i) {
                bgfx::setTransform(glm::value_ptr(mtx));
                // Not sure if this should be part of the material?
                bgfx::setUniform(normalTransformHandle, glm::value_ptr(glm::transpose(glm::inverse(mtx))));

                bgfx::setState(stateTransparent);
                const auto& mesh = m_scene.transparentMeshes.meshes[i];
                bgfx::setIndexBuffer(mesh.indexHandle);
                bgfx::setVertexBuffer(0, mesh.vertexHandle);
                setUniforms(m_scene.transparentMeshes.materials[i], m_scene.transparentMatType);

                bgfx::submit(meshPass, program);
            }

            m_toneMapPass.render(m_pbrFbTextures[0], m_toneMapParams, deltaTime, meshPass + 1);

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

        float m_totalBrightness = 1.0f;

        bae::Scene m_scene;
        bae::SceneUniforms m_sceneUniforms;
        LightSet m_lightSet;

        bgfx::TextureHandle m_pbrFbTextures[2];
        bgfx::FrameBufferHandle m_pbrFramebuffer = BGFX_INVALID_HANDLE;

        bae::ToneMapParams m_toneMapParams;
        bae::ToneMapping m_toneMapPass;

        bool m_computeSupported = true;

        const bgfx::Caps* m_caps;
        float m_time;
    };

}  // namespace example

ENTRY_IMPLEMENT_MAIN(
    example::ExampleForward,
    "02-forward-rendering",
    "Rendering sponza using single lighting pass forward.");
