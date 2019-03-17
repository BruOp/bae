#pragma once
#include <bgfx/bgfx.h>
#include <math.h>
#include <vector>

namespace bae
{
struct Texture {
    bgfx::UniformHandle sampler = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
};

class TextureManager
{
   public:
    TextureManager() = default;
    ~TextureManager()
    {
        for (auto& textureHandle : textureHandles) {
            bgfx::destroy(textureHandle);
        }
    };

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    TextureManager(TextureManager&& other) = default;
    TextureManager& operator=(TextureManager&& other) = default;

    void addTexture(bgfx::TextureHandle texture) { textureHandles.push_back(texture); };

    bgfx::TextureHandle create2DTexture(
        const uint16_t width,
        const uint16_t height,
        const bgfx::TextureFormat::Enum format,
        const bgfx::Memory* mem)
    {
        auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        bgfx::TextureHandle textureHandle = bgfx::createTexture2D(
            width,
            height,
            false,
            mipLevels,
            format,
            BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_MIN_POINT,
            mem);
        textureHandles.push_back(textureHandle);
        return textureHandle;
    }

   private:
    std::vector<bgfx::TextureHandle> textureHandles;
};
}  // namespace bae