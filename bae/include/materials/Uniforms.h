#pragma once
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

namespace bae {
namespace Materials {

    constexpr bgfx::UniformHandle nullUniformHandle{};

    /*
    Lets us create uniform handles, store the data for later use.

    Note that BGFX doesn't support any float, vec2 or vec3 uniforms, see this issue: 
    https://github.com/bkaradzic/bgfx/issues/653

    So if you want to create a Vec2 or Vec3 or float uniform, just use this and only set the
    relevant data. This at least lets you very explicitly control the padding in your memory
    */
    struct Vec4Uniform {
        glm::vec4 data;
        bgfx::UniformHandle handle;

        inline void setUniform() const
        {
            bgfx::setUniform(handle, &(data.x));
        };
    };
}
} // bae
