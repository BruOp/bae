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
            for (auto& texture : textures) {
                bgfx::destroy(texture.sampler);
                bgfx::destroy(texture.handle);
            }
        };

        TextureManager(const TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;

        TextureManager(TextureManager&& other) = default;
        TextureManager& operator=(TextureManager&& other) = default;

        void addTexture(const Texture& texture) {
            textures.push_back(texture);
        };

    private:
        std::vector<Texture> textures;
    };
}