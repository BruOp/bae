#include "Tonemapping.h"

#include <bx/math.h>
#include "bgfx_utils.h"

#include <glm/gtc/type_ptr.hpp>

#include "Offscreen.h"

namespace bae
{
    const std::string ToneMapping::histogramProgramName = "cs_bae_luminance_histogram";
    const std::string ToneMapping::averagingProgramName = "cs_bae_luminance_average";
    const std::string ToneMapping::toneMappingProgramName = "bae_tonemapping";


    void ToneMapping::init()
    {
        histogramProgram = loadProgram(histogramProgramName.c_str(), nullptr);
        averagingProgram = loadProgram(averagingProgramName.c_str(), nullptr);
        std::string vs_name = "vs_" + toneMappingProgramName;
        std::string fs_name = "fs_" + toneMappingProgramName;
        tonemappingProgram = loadProgram(vs_name.c_str(), fs_name.c_str());

        histogramBuffer = bgfx::createDynamicIndexBuffer(
            256,
            BGFX_BUFFER_COMPUTE_READ_WRITE | BGFX_BUFFER_INDEX32
        );

        uint64_t lumAvgFlags = BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP;
        avgLuminanceTarget = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::R16F, lumAvgFlags);
        bgfx::setName(avgLuminanceTarget, "Average Luminance Texture");

        paramsUniform = bgfx::createUniform("u_params", bgfx::UniformType::Vec4);
        s_hdrTexture = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
        s_texAvgLuminance = bgfx::createUniform("s_texAvgLuminance", bgfx::UniformType::Sampler);

        ScreenSpaceQuadVertex::init();

        orthoProjection = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f);
    }

    void ToneMapping::destroy()
    {
        bgfx::destroy(histogramProgram);
        bgfx::destroy(averagingProgram);
        bgfx::destroy(tonemappingProgram);
        bgfx::destroy(histogramBuffer);
        bgfx::destroy(avgLuminanceTarget);
        bgfx::destroy(paramsUniform);
        bgfx::destroy(s_hdrTexture);
        bgfx::destroy(s_texAvgLuminance);

    }

    void ToneMapping::render(bgfx::TextureHandle hdrFbTexture, const ToneMapParams& toneMapParams, const float deltaTime)
    {
        bgfx::ViewId histogramPass = 1;
        bgfx::ViewId averagingPass = 2;
        bgfx::ViewId tonemapPass = 3;

        bgfx::setViewName(histogramPass, "Luminence Histogram");
        bgfx::setViewName(averagingPass, "Avergaing the Luminence Histogram");

        bgfx::setViewName(tonemapPass, "Tonemap");
        bgfx::setViewRect(tonemapPass, 0, 0, bgfx::BackbufferRatio::Equal);
        bgfx::setViewFrameBuffer(tonemapPass, BGFX_INVALID_HANDLE);

        for (bgfx::ViewId i = histogramPass; i <= tonemapPass; i++) {
            bgfx::setViewTransform(i, nullptr, glm::value_ptr(orthoProjection));
        }

        float params[4] = {
            toneMapParams.minLogLuminance,
            1.0f / (toneMapParams.maxLogLuminance - toneMapParams.minLogLuminance),
            0.0f,
            0.0f
        };
        uint32_t groupsX = static_cast<uint32_t>(bx::ceil(toneMapParams.width / 16.0f));
        uint32_t groupsY = static_cast<uint32_t>(bx::ceil(toneMapParams.height / 16.0f));
        bgfx::setUniform(paramsUniform, params);
        bgfx::setImage(0, hdrFbTexture, 0, bgfx::Access::Read, frameBufferFormat);
        bgfx::setBuffer(1, histogramBuffer, bgfx::Access::Write);
        bgfx::dispatch(histogramPass, histogramProgram, groupsX, groupsY, 1);

        float timeCoeff = bx::clamp<float>(1.0f - bx::exp(-deltaTime * toneMapParams.tau), 0.0, 1.0);
        params[1] = toneMapParams.maxLogLuminance - toneMapParams.minLogLuminance;
        params[2] = timeCoeff;
        params[3] = static_cast<float>(toneMapParams.width * toneMapParams.height);
        bgfx::setUniform(paramsUniform, params);
        bgfx::setImage(0, avgLuminanceTarget, 0, bgfx::Access::ReadWrite, bgfx::TextureFormat::R16F);
        bgfx::setBuffer(1, histogramBuffer, bgfx::Access::ReadWrite);
        bgfx::dispatch(averagingPass, averagingProgram, 1, 1, 1);


        float tonemap[4] = { 0.18f, bx::square(3.0f), 0.0f, 0.0f };
        bgfx::setTexture(0, s_hdrTexture, hdrFbTexture);
        bgfx::setTexture(1, s_texAvgLuminance, avgLuminanceTarget, BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP);
        bgfx::setUniform(paramsUniform, tonemap);
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
        setScreenSpaceQuad((float)toneMapParams.width, (float)toneMapParams.height, toneMapParams.originBottomLeft);
        bgfx::submit(tonemapPass, tonemappingProgram);
    }
}