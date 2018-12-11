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
        Vec4Uniform()
        {
            handle.idx = bgfx::kInvalidHandle;
        };
        Vec4Uniform(const std::string& name, const glm::vec4& data)
            : name{ name }
            , handle{ bgfx::createUniform(name.c_str(), bgfx::UniformType::Vec4) }
            , data{ data } {};

        ~Vec4Uniform()
        {
            if (bgfx::isValid(handle)) {
                destroy();
            }
        };

        Vec4Uniform(const Vec4Uniform&) = delete;
        Vec4Uniform& operator=(const Vec4Uniform&) = delete;

        Vec4Uniform(Vec4Uniform&& other)
            : name{ other.name }
            , handle{ other.handle }
            , data{ other.data }
        {
            other.handle.idx = bgfx::kInvalidHandle;
        };

        Vec4Uniform& operator=(Vec4Uniform&& other)
        {
            if (this != &other) {
                name = other.name;
                data = other.data;
                handle = other.handle;

                other.handle.idx = bgfx::kInvalidHandle;
            }
            return *this;
        }

        inline void destroy()
        {
            bgfx::destroy(handle);
        }

        inline void setUniform() const
        {
            bgfx::setUniform(handle, &(data.x));
        };

        std::string name;
        bgfx::UniformHandle handle;
        glm::vec4 data;
    };
}
} // bae
