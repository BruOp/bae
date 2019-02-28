#pragma once
#include <vector>
#include <bgfx/bgfx.h>

namespace bae {
    struct Texture {
        bgfx::UniformHandle sampler = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
    };

    class TextureManager {
    public:
        TextureManager() = default;
        ~TextureManager() {
            for (auto& textureHandle : textureHandles) {
                bgfx::destroy(textureHandle);
            }
        };

        TextureManager(const TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;

        TextureManager(TextureManager&& other) = default;
        TextureManager& operator=(TextureManager&& other) = default;

        void addTexture(bgfx::TextureHandle texture) {
            textureHandles.push_back(texture);
        };

        bgfx::TextureHandle create2DTexture(const uint16_t width, const uint16_t height, const bgfx::TextureFormat::Enum format, const bgfx::Memory* mem) {
            bgfx::TextureHandle textureHandle = bgfx::createTexture2D(
                width,
                height,
                false,
                1,
                format,
                0,
                mem
            );
            textureHandles.push_back(textureHandle);
            return textureHandle;
        }

    private:
        std::vector<bgfx::TextureHandle> textureHandles;
    };
}