#include <string>

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

namespace bae
{
    // We need a frame buffer to hold the output of the regular render pass.
    // Lets assume that's supplied from our renderer

    // We'll also need a histogram program and dynamic buffer
    // That program also requires a uniform
    struct ToneMapParams
    {
        uint32_t width;
        uint32_t height;
        float minLogLuminance = -8.0f;
        float maxLogLuminance = 3.0f;
        float tau = 1.1f;
        bool originBottomLeft = false;
    };

    struct ToneMapping
    {
        bgfx::ProgramHandle histogramProgram = BGFX_INVALID_HANDLE;
        bgfx::ProgramHandle averagingProgram = BGFX_INVALID_HANDLE;
        bgfx::ProgramHandle tonemappingProgram = BGFX_INVALID_HANDLE;
        bgfx::DynamicIndexBufferHandle histogramBuffer = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle avgLuminanceTarget = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle paramsUniform = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle s_hdrTexture = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle s_texAvgLuminance = BGFX_INVALID_HANDLE;

        float orthoProjection[16];

        static constexpr bgfx::TextureFormat::Enum frameBufferFormat = bgfx::TextureFormat::RGBA16F;
        static const std::string histogramProgramName;
        static const std::string averagingProgramName;
        static const std::string toneMappingProgramName;

        void init(const bgfx::Caps* caps);

        void destroy();

        void render(bgfx::TextureHandle hdrFbTexture, const ToneMapParams& toneMapParams, const float deltaTime, bgfx::ViewId startingPass);
    };
}
