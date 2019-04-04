#pragma once
#include <bgfx/bgfx.h>

#include "Camera.h"
#include "Uniforms.h"

namespace bae
{
struct SceneUniforms {
    bgfx::UniformHandle cameraPos = BGFX_INVALID_HANDLE;

    void init() { cameraPos = bgfx::createUniform("cameraPos", bgfx::UniformType::Vec4); };

    void destroy() { bgfx::destroy(cameraPos); }

    inline void setCamera(const Camera& camera, const bgfx::ViewId viewId) const
    {
        bgfx::setViewTransform(viewId, glm::value_ptr(camera.view), glm::value_ptr(camera.projection));
        bgfx::setUniform(cameraPos, &(camera.position[0]));
    }
};
}  // namespace bae
