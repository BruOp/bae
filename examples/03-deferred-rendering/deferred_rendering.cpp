/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <iostream>
#include <array>
#include <random>
#include <bx/rng.h>
#include "bgfx_utils.h"
#include "common.h"
#include "imgui/imgui.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "bae/PhysicallyBasedScene.h"
#include "bae/gltf_model_loading.h"
#include "bae/Tonemapping.h"
#include "bae/Offscreen.h"
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

    std::vector<glm::vec3> sampleUnitCylinderUniformly(size_t N) {
        std::default_random_engine generator(10);
        std::uniform_real_distribution<float> rand(0.0f, 1.0f);
        std::vector<glm::vec3> output(N);
        for (size_t i = 0; i < N; ++i) {
            output[i] = glm::vec3{
                sqrt(rand(generator)),
                rand(generator) * 2 * bx::kPi,
                rand(generator),
            };
        }
        return output;
    }

    class LightSet {
    public:
        size_t numActiveLights;
        size_t maxNumLights = 2048;
        bae::Mesh volumeMesh;
        std::vector<glm::vec3> initialPositions;
        std::vector<glm::vec4> positionRadiusData;
        std::vector<glm::vec4> colorIntensityData;

        void init()
        {
            bae::IcosahedronFactory factory{ 2 };
            volumeMesh = factory.getMesh();
            initialPositions = sampleUnitCylinderUniformly(maxNumLights);
            positionRadiusData.resize(maxNumLights);
            colorIntensityData.resize(maxNumLights);
            size_t numColors = LIGHT_COLORS.size();
            // Initialize color data for all lights
            for (size_t i = 0; i < maxNumLights; i++) {
                colorIntensityData[i] = glm::vec4{ LIGHT_COLORS[i % numColors], 1.0f };
            }
        }

        void destroy()
        {
            bae::destroy(volumeMesh);
        }
    };

    struct PBRShaderUniforms {
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
        // We pack our baseColorFactor, emissiveFactor, roughnessFactor and metallicFactor into this uniform
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

    void bindUniforms(const PBRShaderUniforms& uniforms, const bae::PBRMaterial& material, const glm::mat4& transform) {
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

    struct DeferredSceneUniforms {
        bgfx::UniformHandle s_baseColorRoughness = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle s_normalMetallic = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle s_emissiveOcclusion = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle s_depth = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle u_cameraPos = BGFX_INVALID_HANDLE;
    };

    void init(DeferredSceneUniforms& uniforms) {
        uniforms.s_baseColorRoughness = bgfx::createUniform("s_baseColorRoughness", bgfx::UniformType::Sampler);
        uniforms.s_normalMetallic = bgfx::createUniform("s_normalMetallic", bgfx::UniformType::Sampler);
        uniforms.s_emissiveOcclusion = bgfx::createUniform("s_emissiveOcclusion", bgfx::UniformType::Sampler);
        uniforms.s_depth = bgfx::createUniform("s_depth", bgfx::UniformType::Sampler);
        uniforms.u_cameraPos = bgfx::createUniform("u_cameraPos", bgfx::UniformType::Vec4);
    }

    void destroy(DeferredSceneUniforms& uniforms) {
        bgfx::destroy(uniforms.s_baseColorRoughness);
        bgfx::destroy(uniforms.s_normalMetallic);
        bgfx::destroy(uniforms.s_emissiveOcclusion);
        bgfx::destroy(uniforms.s_depth);
        bgfx::destroy(uniforms.u_cameraPos);
    }

    struct PointLightUniforms {
        bgfx::UniformHandle u_lightColorIntensity = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle u_lightPosRadius = BGFX_INVALID_HANDLE;
    };

    void init(PointLightUniforms& uniforms) {
        uniforms.u_lightColorIntensity = bgfx::createUniform("u_lightColorIntensity", bgfx::UniformType::Vec4);
        uniforms.u_lightPosRadius = bgfx::createUniform("u_lightPosRadius", bgfx::UniformType::Vec4);
    }

    void destroy(PointLightUniforms& uniforms) {
        bgfx::destroy(uniforms.u_lightColorIntensity);
        bgfx::destroy(uniforms.u_lightPosRadius);
    }

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

            m_writeToRTProgram = loadProgram("vs_deferred_pbr", "fs_deferred_pbr");
            m_lightStencilProgram = loadProgram("vs_light_stencil", "fs_light_stencil");
            m_pointLightVolumeProgram = loadProgram("vs_point_light_volume", "fs_point_light_volume");
            m_emissivePassProgram = loadProgram("vs_emissive_pass", "fs_emissive_pass");

            example::init(m_pbrUniforms);
            example::init(m_deferredSceneUniforms);
            example::init(m_pointLightUniforms);

            // Lets load all the meshes
            m_model = bae::loadGltfModel("meshes/Sponza/", "Sponza.gltf");

            m_lightSet.init();
            m_lightSet.numActiveLights = 256;
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
            if (m_computeSupported) {
                if (bgfx::isValid(m_hdrFrameBuffer))
                {
                    bgfx::destroy(m_hdrFrameBuffer);
                }

                if (bgfx::isValid(m_gBuffer))
                {
                    bgfx::destroy(m_gBuffer);
                }

                if (bgfx::isValid(m_lightGBuffer)) {
                    bgfx::destroy(m_lightGBuffer);
                }

                m_toneMapPass.destroy();

                // Cleanup.
                destroy(m_pbrUniforms);
                destroy(m_deferredSceneUniforms);
                destroy(m_pointLightUniforms);
                bgfx::destroy(m_writeToRTProgram);
                bgfx::destroy(m_lightStencilProgram);
                bgfx::destroy(m_pointLightVolumeProgram);
                bgfx::destroy(m_emissivePassProgram);
                destroy(m_model);
                m_lightSet.destroy();

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
                m_gbufferTex[5].idx = bgfx::kInvalidHandle;
            }

            const uint64_t tsFlags = 0
                | BGFX_SAMPLER_MIN_POINT
                | BGFX_SAMPLER_MAG_POINT
                | BGFX_SAMPLER_MIP_POINT
                | BGFX_SAMPLER_U_CLAMP
                | BGFX_SAMPLER_V_CLAMP
                ;

            bgfx::Attachment gbufferAt[6] = {};

            // The gBuffers store the following, in order:
            // - RGB - Base Color, A -Roughness
            // - RGB16 - Normal, A - Metalness
            // - RGB - Emissive, A - Occlusion
            // - R32F - Depth
            // - RGBA16F - Final Radiance
            m_gbufferTex[0] = bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT | tsFlags);
            m_gbufferTex[1] = bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::RGBA16F, BGFX_TEXTURE_RT | tsFlags);
            m_gbufferTex[2] = bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT | tsFlags);
            m_gbufferTex[3] = bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::R32F, BGFX_TEXTURE_RT | tsFlags);
            m_gbufferTex[4] = bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::RGBA16F, BGFX_TEXTURE_RT | tsFlags);

            gbufferAt[0].init(m_gbufferTex[0]);
            gbufferAt[1].init(m_gbufferTex[1]);
            gbufferAt[2].init(m_gbufferTex[2]);
            gbufferAt[3].init(m_gbufferTex[3]);
            gbufferAt[4].init(m_gbufferTex[4]);

            m_gbufferTex[5] = bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT | tsFlags);
            gbufferAt[5].init(m_gbufferTex[5]);

            m_gBuffer = bgfx::createFrameBuffer(BX_COUNTOF(gbufferAt), gbufferAt, true);
            m_lightGBuffer = bgfx::createFrameBuffer(2, &gbufferAt[4], false);

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

            int numActiveLights = int32_t(m_lightSet.numActiveLights);
            ImGui::SliderInt("Num lights", &numActiveLights, 1, int(m_lightSet.maxNumLights));
            ImGui::DragFloat("Total Brightness", &m_totalBrightness, 0.5f, 0.0f, 250.0f);

            ImGui::End();

            imguiEndFrame();

            bgfx::ViewId meshPass = 0;
            bgfx::setViewRect(meshPass, 0, 0, uint16_t(m_width), uint16_t(m_height));
            bgfx::setViewFrameBuffer(meshPass, m_gBuffer);
            bgfx::setViewName(meshPass, "Draw Meshes");

            bgfx::ViewId lightingPass = 1;
            bgfx::setViewFrameBuffer(lightingPass, m_lightGBuffer);
            bgfx::setViewRect(lightingPass, 0, 0, uint16_t(m_width), uint16_t(m_height));
            // We want our draw calls to execute in order
            bgfx::setViewMode(lightingPass, bgfx::ViewMode::Sequential);
            bgfx::setViewName(lightingPass, "Lighting Pass");

            bgfx::ViewId emissivePass = 2;
            bgfx::setViewFrameBuffer(emissivePass, m_lightGBuffer);
            bgfx::setViewRect(emissivePass, 0, 0, uint16_t(m_width), uint16_t(m_height));
            bgfx::setViewName(emissivePass, "Emissive Pass");


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

            cameraUpdate(0.1f * deltaTime, m_mouseState);
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
            bgfx::setUniform(m_deferredSceneUniforms.u_cameraPos, &cameraPos.x);

            // Render all our opaque meshes
            const bae::MeshGroup& meshes = m_model.opaqueMeshes;
            for (size_t i = 0; i < meshes.meshes.size(); ++i) {
                const auto& mesh = meshes.meshes[i];
                const auto& transform = meshes.transforms[i];
                const auto& material = meshes.materials[i];

                bgfx::setState(stateOpaque);
                bindUniforms(m_pbrUniforms, material, transform);
                mesh.setBuffers();
                bgfx::submit(meshPass, m_writeToRTProgram);
            }

            // Render all our masked meshes
            const bae::MeshGroup& maskedMeshes = m_model.maskedMeshes;
            for (size_t i = 0; i < maskedMeshes.meshes.size(); ++i) {
                const auto& mesh = maskedMeshes.meshes[i];
                const auto& transform = maskedMeshes.transforms[i];
                const auto& material = maskedMeshes.materials[i];

                bgfx::setState(stateOpaque);
                bindUniforms(m_pbrUniforms, material, transform);
                mesh.setBuffers();
                bgfx::submit(meshPass, m_writeToRTProgram);
            }

            {
                m_lightSet.numActiveLights = uint16_t(numActiveLights);
                // Move our lights around in a cylinder the size of our scene
                // These numbers are adhoc, based on sponza.
                constexpr float sceneWidth = 12.0f;
                constexpr float sceneLength = 4.0f;
                constexpr float sceneHeight = 10.0f;
                constexpr float timeCoeff = 0.3f;

                float N = float(m_lightSet.numActiveLights);
                float intensity = m_totalBrightness / N;
                constexpr float EPSILON = 0.01f;
                float radius = bx::sqrt(intensity / EPSILON);

                for (size_t i = 0; i < m_lightSet.numActiveLights; ++i) {
                    glm::vec3& initial = m_lightSet.initialPositions[i];
                    float r = initial.x;
                    float phaseOffset = initial.y;
                    float z = initial.z;
                    // Set positions in Cartesian
                    m_lightSet.positionRadiusData[i].x = r * sceneWidth * bx::cos(timeCoeff * m_time + phaseOffset);
                    m_lightSet.positionRadiusData[i].z = r * sceneLength * bx::sin(timeCoeff * m_time + phaseOffset);
                    m_lightSet.positionRadiusData[i].y = sceneHeight * z;
                    // Set intensity and radius
                    m_lightSet.colorIntensityData[i].w = intensity;
                    m_lightSet.positionRadiusData[i].w = radius;
                }
            }

            bgfx::setViewTransform(lightingPass, view, proj);
            // We need to set up the stencil state for rendering our lights
            uint64_t stencilState = 0
                | BGFX_STATE_DEPTH_TEST_LESS;
            // Lets render our light volumes
            for (size_t i = 0; i < m_lightSet.numActiveLights; ++i) {
                // First, we render our light volumes purely to determine stencil state
                // We determine whether a light volume should be rendered by the following algo:
                //   1) The front faces must be IN FRONT of scene geometry
                //   2) The back faces must be BEHIND scene geometry
                // However, our stencil test is setup such that any non-zero value is considered
                // a FAIL -- so we increment whenever a fragement in our light volume fails to
                // satisfy either condition.
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
                glm::vec3 position{ m_lightSet.positionRadiusData[i].x, m_lightSet.positionRadiusData[i].y, m_lightSet.positionRadiusData[i].z };
                glm::vec3 scale{ m_lightSet.positionRadiusData[i].w };
                modelTransform = glm::scale(
                    glm::translate(modelTransform, position),
                    scale
                );

                bgfx::setTransform(glm::value_ptr(modelTransform));
                bgfx::setState(stencilState);
                bgfx::setStencil(frontStencilFunc, backStencilFunc);
                m_lightSet.volumeMesh.setBuffers();
                bgfx::submit(lightingPass, m_lightStencilProgram);

                // Now that we have incremented our stencil for fragments we DONT want to shade,
                // lets perform the shading pass with our stencil test set such that it must equal 0
                // Additionally, let's "clean up" after ourselves by reseting the failing, incremented
                // stencil fragments back to zero.
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
                bgfx::setState(lightVolumeState);
                bgfx::setStencil(frontStencilFunc, backStencilFunc);
                m_lightSet.volumeMesh.setBuffers();
                bgfx::setTexture(0, m_deferredSceneUniforms.s_baseColorRoughness, m_gbufferTex[0], BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP);
                bgfx::setTexture(1, m_deferredSceneUniforms.s_normalMetallic, m_gbufferTex[1], BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP);
                bgfx::setTexture(2, m_deferredSceneUniforms.s_emissiveOcclusion, m_gbufferTex[2], BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP);
                bgfx::setTexture(3, m_deferredSceneUniforms.s_depth, m_gbufferTex[3], BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP);
                bgfx::setUniform(m_pointLightUniforms.u_lightPosRadius, glm::value_ptr(m_lightSet.positionRadiusData[i]));
                bgfx::setUniform(m_pointLightUniforms.u_lightColorIntensity, glm::value_ptr(m_lightSet.colorIntensityData[i]));
                bgfx::submit(lightingPass, m_pointLightVolumeProgram);
            }

            // Here we are simply drawing a full screen quad to add emissive radiance from our gbuffers into our final buffer
            float orthoProjection[16];
            bx::mtxOrtho(orthoProjection, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 2.0f, 0.0f, m_caps->homogeneousDepth);
            uint64_t emissivePassState = 0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_BLEND_ADD;
            bgfx::setState(emissivePassState);
            bae::setScreenSpaceQuad(float(m_width), float(m_height), m_caps->originBottomLeft);
            bgfx::setViewTransform(emissivePass, nullptr, orthoProjection);
            bgfx::setTexture(0, m_deferredSceneUniforms.s_emissiveOcclusion, m_gbufferTex[2], BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP);
            bgfx::submit(emissivePass, m_emissivePassProgram);

            m_toneMapPass.render(m_gbufferTex[4], m_toneMapParams, deltaTime, emissivePass + 1);

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

        bgfx::ProgramHandle m_writeToRTProgram;
        bgfx::ProgramHandle m_lightStencilProgram;
        bgfx::ProgramHandle m_pointLightVolumeProgram;
        bgfx::ProgramHandle m_emissivePassProgram;

        bae::Model m_model;
        PBRShaderUniforms m_pbrUniforms;
        DeferredSceneUniforms m_deferredSceneUniforms;
        PointLightUniforms m_pointLightUniforms;

        LightSet m_lightSet;

        float m_totalBrightness = 1.0f;
        // Deferred passes
        bgfx::FrameBufferHandle m_gBuffer = BGFX_INVALID_HANDLE;
        bgfx::FrameBufferHandle m_lightGBuffer = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle m_gbufferTex[6];

        // Buffer to put final outputs into
        bgfx::TextureHandle m_hdrFbTextures[2];
        bgfx::FrameBufferHandle m_hdrFrameBuffer = BGFX_INVALID_HANDLE;

        bae::ToneMapParams m_toneMapParams;
        bae::ToneMapping m_toneMapPass;

        const bgfx::Caps* m_caps;
        float m_time;

        bool m_computeSupported = true;
    };

}  // namespace example

ENTRY_IMPLEMENT_MAIN(
    example::ExampleDeferred,
    "03-deferred-rendering",
    "Rendering sponza using deferred rendering.");
