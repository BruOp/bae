#pragma once
#include <bgfx/bgfx.h>

#include "Camera.h"
#include "materials/Uniforms.h"

namespace bae {
struct SceneUniforms {
    bgfx::UniformHandle cameraPos = BGFX_INVALID_HANDLE;

    void init()
    {
        cameraPos = bgfx::createUniform("cameraPos", bgfx::UniformType::Vec4);
    };

    void destroy()
    {
        bgfx::destroy(cameraPos);
    }

    inline void setCamera(const Camera& camera) const
    {
        bgfx::setUniform(cameraPos, &(camera.position[0]));
    }
};
}
