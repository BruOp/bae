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
#include <random>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "bae/PhysicallyBasedScene.h"
#include "bae/Tonemapping.h"
#include "bae/gltf_model_loading.h"

namespace example
{
#define SAMPLER_POINT_CLAMP (BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP)

    static float s_texelHalf = 0.0f;

    struct DirectionalLight
    {
        glm::vec3 m_color = glm::vec3{ 1.0f, 1.0f, 1.0f };
        float m_intensity = 10.0f;
        glm::vec4 m_direction = glm::normalize(glm::vec4{ 1.0, 1.0f, -1.0f, 0.0 });

        bgfx::UniformHandle u_directionalLightParams;
    };

    void init(DirectionalLight& light)
    {
        light.u_directionalLightParams = bgfx::createUniform("u_directionalLightParams", bgfx::UniformType::Vec4, 2);
    }

    void destroy(DirectionalLight& light) {
        bgfx::destroy(light.u_directionalLightParams);
    }

    void bindUniforms(const DirectionalLight& light) {
        bgfx::setUniform(light.u_directionalLightParams, &light, 2);
    }

    struct PBRShaderUniforms
    {
        bgfx::UniformHandle s_baseColor = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle s_normal = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle s_metallicRoughness = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle s_emissive = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle s_occlusion = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle u_factors = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle u_normalTransform = BGFX_INVALID_HANDLE;
    };

    void init(PBRShaderUniforms& uniforms)
    {
        uniforms.s_baseColor = bgfx::createUniform("s_baseColor", bgfx::UniformType::Sampler);
        uniforms.s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Sampler);
        uniforms.s_metallicRoughness = bgfx::createUniform("s_metallicRoughness", bgfx::UniformType::Sampler);
        uniforms.s_emissive = bgfx::createUniform("s_emissive", bgfx::UniformType::Sampler);
        uniforms.s_occlusion = bgfx::createUniform("s_occlusion", bgfx::UniformType::Sampler);
        // We are going to pack our baseColorFactor, emissiveFactor, roughnessFactor
        // and metallicFactor into this uniform
        uniforms.u_factors = bgfx::createUniform("u_factors", bgfx::UniformType::Vec4, 3);
        uniforms.u_normalTransform = bgfx::createUniform("u_normalTransform", bgfx::UniformType::Mat4);
    }

    void destroy(PBRShaderUniforms& uniforms)
    {
        bgfx::destroy(uniforms.s_baseColor);
        bgfx::destroy(uniforms.s_normal);
        bgfx::destroy(uniforms.s_metallicRoughness);
        bgfx::destroy(uniforms.s_emissive);
        bgfx::destroy(uniforms.s_occlusion);
        bgfx::destroy(uniforms.u_factors);
        bgfx::destroy(uniforms.u_normalTransform);
    }

    void bindUniforms(const PBRShaderUniforms& uniforms, const bae::PBRMaterial& material, const glm::mat4& transform)
    {
        bgfx::setTexture(0, uniforms.s_baseColor, material.baseColorTexture);
        bgfx::setTexture(1, uniforms.s_normal, material.normalTexture);
        bgfx::setTexture(2, uniforms.s_metallicRoughness, material.metallicRoughnessTexture);
        bgfx::setTexture(3, uniforms.s_emissive, material.emissiveTexture);
        bgfx::setTexture(4, uniforms.s_occlusion, material.occlusionTexture);
        // We are going to pack our baseColorFactor, emissiveFactor, roughnessFactor
        // and metallicFactor into this uniform
        bgfx::setUniform(uniforms.u_factors, &material.baseColorFactor, 3);

        // Transforms
        bgfx::setTransform(glm::value_ptr(transform));
        glm::mat4 normalTransform{ glm::transpose(glm::inverse(transform)) };
        bgfx::setUniform(uniforms.u_normalTransform, glm::value_ptr(normalTransform));
    }

    struct SceneUniforms
    {
        bgfx::UniformHandle u_cameraPos = BGFX_INVALID_HANDLE;
    };

    void init(SceneUniforms& uniforms)
    {
        uniforms.u_cameraPos = bgfx::createUniform("u_cameraPos", bgfx::UniformType::Vec4);
    }

    void destroy(SceneUniforms& uniforms) {
        bgfx::destroy(uniforms.u_cameraPos);
    }

    void bindUniforms(const SceneUniforms& uniforms, const bx::Vec3 cameraPos)
    {
        bgfx::setUniform(uniforms.u_cameraPos, &cameraPos);
    }

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

            m_caps = bgfx::getCaps();
            m_computeSupported = !!(m_caps->supported & BGFX_CAPS_COMPUTE);

            if (!m_computeSupported)
            {
                return;
            }

            m_directionalShadowMapProgram = loadProgram("vs_directional_shadowmap", "fs_directional_shadowmap");
            m_prepassProgram = loadProgram("vs_z_prepass", "fs_z_prepass");
            m_pbrShader = loadProgram("vs_pbr", "fs_pbr");
            m_pbrShaderWithMasking = loadProgram("vs_pbr", "fs_pbr_masked");

            // Lets load all the meshes
            m_model = bae::loadGltfModel("meshes/Sponza/", "Sponza.gltf");

            example::init(m_pbrUniforms);
            example::init(m_sceneUniforms);
            example::init(m_directionalLight);

            m_totalBrightness = 100.0f;

            m_toneMapParams.width = m_width;
            m_toneMapParams.width = m_height;
            m_toneMapParams.originBottomLeft = m_caps->originBottomLeft;
            m_toneMapPass.init(m_caps);

            // Imgui.
            imguiCreate();

            s_texelHalf = bgfx::RendererType::Direct3D9 == m_caps->rendererType ? 0.5f : 0.0f;

            // Init camera
            cameraCreate();
            cameraSetPosition({ 0.0f, 2.0f, 0.0f });
            cameraSetHorizontalAngle(bx::kPi / 2.0);
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

            if (bgfx::isValid(m_pbrFramebuffer))
            {
                bgfx::destroy(m_pbrFramebuffer);
            }

            if (bgfx::isValid(m_shadowMapFramebuffer))
            {
                bgfx::destroy(m_shadowMapFramebuffer);
            }

            m_toneMapPass.destroy();

            // Cleanup.
            destroy(m_directionalLight);
            destroy(m_pbrUniforms);
            destroy(m_sceneUniforms);
            bae::destroy(m_model);
            bgfx::destroy(m_directionalShadowMapProgram);
            bgfx::destroy(m_prepassProgram);
            bgfx::destroy(m_pbrShader);
            bgfx::destroy(m_pbrShaderWithMasking);

            cameraDestroy();

            imguiDestroy();
            // Shutdown bgfx.
            bgfx::shutdown();

            return 0;
        }

        void renderMeshes(
            const bae::MeshGroup& meshes,
            const bx::Vec3& cameraPos,
            const uint64_t state,
            const bgfx::ProgramHandle program,
            const bgfx::ViewId viewId) const
        {
            // Render all our opaque meshes
            for (size_t i = 0; i < meshes.meshes.size(); ++i)
            {
                const auto& mesh = meshes.meshes[i];
                const auto& transform = meshes.transforms[i];
                const auto& material = meshes.materials[i];

                bgfx::setState(state);
                bindUniforms(m_pbrUniforms, material, transform);
                bindUniforms(m_sceneUniforms, cameraPos);
                bindUniforms(m_directionalLight);
                mesh.setBuffers();

                bgfx::submit(viewId, program);
            }
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

            if (!bgfx::isValid(m_pbrFramebuffer) || m_oldWidth != m_width || m_oldHeight != m_height || m_oldReset != m_reset)
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
                    uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::RGBA16F, (uint64_t(msaa + 1) << BGFX_TEXTURE_RT_MSAA_SHIFT) | BGFX_SAMPLER_UVW_CLAMP | BGFX_SAMPLER_POINT);

                const uint64_t textureFlags = BGFX_TEXTURE_RT_WRITE_ONLY | (uint64_t(msaa + 1) << BGFX_TEXTURE_RT_MSAA_SHIFT);

                bgfx::TextureFormat::Enum depthFormat;
                if (bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, textureFlags))
                {
                    depthFormat = bgfx::TextureFormat::D24S8;
                }
                else
                {
                    depthFormat = bgfx::TextureFormat::D32;
                }

                m_pbrFbTextures[1] = bgfx::createTexture2D(
                    uint16_t(m_width), uint16_t(m_height), false, 1, depthFormat, textureFlags);

                bgfx::setName(m_pbrFbTextures[0], "HDR Buffer");

                m_pbrFramebuffer = bgfx::createFrameBuffer(BX_COUNTOF(m_pbrFbTextures), m_pbrFbTextures, true);
            }

            imguiBeginFrame(m_mouseState.m_mx, m_mouseState.m_my, (m_mouseState.m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0) | (m_mouseState.m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0) | (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0), m_mouseState.m_mz, uint16_t(m_width), uint16_t(m_height));

            showExampleDialog(this);

            ImGui::SetNextWindowPos(
                ImVec2(m_width - m_width / 5.0f - 10.0f, 10.0f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(
                ImVec2(m_width / 5.0f, m_height / 3.0f), ImGuiCond_FirstUseEver);
            ImGui::Begin("Settings", NULL, 0);

            ImGui::DragFloat("Total Brightness", &m_totalBrightness, 0.5f, 0.0f, 250.0f);
            ImGui::Checkbox("Z-Prepass Enabled", &m_zPrepassEnabled);

            ImGui::End();

            imguiEndFrame();

            bgfx::ViewId zPrepass = 0;
            bgfx::setViewFrameBuffer(zPrepass, m_pbrFramebuffer);
            bgfx::setViewName(zPrepass, "Z Prepass");
            bgfx::setViewRect(zPrepass, 0, 0, uint16_t(m_width), uint16_t(m_height));

            bgfx::ViewId meshPass = 1;
            bgfx::setViewFrameBuffer(meshPass, m_pbrFramebuffer);
            bgfx::setViewName(meshPass, "Draw Meshes");
            bgfx::setViewRect(meshPass, 0, 0, uint16_t(m_width), uint16_t(m_height));

            // This dummy draw call is here to make sure that view 0 is cleared
            // if no other draw calls are submitted to view 0.
            if (m_zPrepassEnabled)
            {
                bgfx::setViewClear(zPrepass, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x030303ff, 1.0f, 0);
                bgfx::setViewClear(meshPass, 0);
                bgfx::touch(zPrepass);
            }
            else
            {
                bgfx::setViewClear(meshPass, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x030303ff, 1.0f, 0);
                bgfx::touch(meshPass);
            }

            int64_t now = bx::getHPCounter();
            static int64_t last = now;
            const int64_t frameTime = now - last;
            last = now;
            const double freq = double(bx::getHPFrequency());
            const float deltaTime = (float)(frameTime / freq);
            m_time += deltaTime;

            float proj[16];
            bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);

            // Update camera
            float view[16];

            cameraUpdate(0.5f * deltaTime, m_mouseState);
            cameraGetViewMtx(view);
            // Set view and projection matrix
            bgfx::setViewTransform(zPrepass, view, proj);
            bgfx::setViewTransform(meshPass, view, proj);

            // Not scaling or translating our scene
            glm::mat4 mtx = glm::identity<glm::mat4>();

            // Set view 0 default viewport.
            bx::Vec3 cameraPos = cameraGetPosition();

            uint64_t stateOpaque = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;

            if (m_zPrepassEnabled)
            {
                stateOpaque |= BGFX_STATE_DEPTH_TEST_LEQUAL;
            }
            else
            {
                stateOpaque |= BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS;
            }

            uint64_t stateTransparent = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_BLEND_ALPHA;

            if (m_zPrepassEnabled)
            {
                uint64_t statePrepass = 0 | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;

                // Render all our opaque meshes
                renderMeshes(m_model.opaqueMeshes, cameraPos, statePrepass, m_pbrShader, zPrepass);
            }

            // Render all our opaque meshes
            renderMeshes(m_model.opaqueMeshes, cameraPos, stateOpaque, m_pbrShader, meshPass);

            // Render all our masked meshes
            renderMeshes(m_model.maskedMeshes, cameraPos, stateOpaque & ~BGFX_STATE_WRITE_Z, m_pbrShaderWithMasking, meshPass);

            // Render all our transparent meshes
            renderMeshes(m_model.transparentMeshes, cameraPos, stateTransparent, m_pbrShader, meshPass);

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
        float m_time;

        bgfx::ProgramHandle m_directionalShadowMapProgram;
        bgfx::ProgramHandle m_prepassProgram;
        bgfx::ProgramHandle m_pbrShader;
        bgfx::ProgramHandle m_pbrShaderWithMasking;


        bgfx::TextureHandle m_shadowMapTextures[2];
        bgfx::FrameBufferHandle m_shadowMapFramebuffer = BGFX_INVALID_HANDLE;

        bgfx::TextureHandle m_pbrFbTextures[2];
        bgfx::FrameBufferHandle m_pbrFramebuffer = BGFX_INVALID_HANDLE;

        PBRShaderUniforms m_pbrUniforms = {};
        SceneUniforms m_sceneUniforms = {};
        
        bae::Model m_model;

        DirectionalLight m_directionalLight = {};

        bae::ToneMapParams m_toneMapParams;
        bae::ToneMapping m_toneMapPass;

        const bgfx::Caps* m_caps;

        bool m_computeSupported = true;
        bool m_zPrepassEnabled = false;
    };

} // namespace example

ENTRY_IMPLEMENT_MAIN(
    example::ExampleForward,
    "05-shadow-mapping",
    "Rendering sponza with directional lighting and cascaded shadow maps.");
