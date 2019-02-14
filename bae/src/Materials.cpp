#include "Materials.h"

namespace bae {
namespace Materials {

    MaterialType Basic::materialType = {
        "basic",
        BGFX_INVALID_HANDLE,
        {
            { "matColor", { bgfx::UniformType::Vec4 } },
        }
    };

    MaterialType Lambertian::materialType = {
        "lambertian",
        BGFX_INVALID_HANDLE,
        {
            { "matColor", { bgfx::UniformType::Vec4 } },
        }
    };

    MaterialType Physical::materialType = {
        "physical",
        BGFX_INVALID_HANDLE,
        {
            { "matColor", { bgfx::UniformType::Vec4 } },
            { "metallicRoughnessReflectance", { bgfx::UniformType::Vec4 } },
        }
    };
}
}
