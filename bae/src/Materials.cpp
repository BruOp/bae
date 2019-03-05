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

    MaterialType TexturedBasic::materialType = {
        "textured_basic",
        BGFX_INVALID_HANDLE,
        {
            { "baseColor", { bgfx::UniformType::Int1 } },
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

    MaterialType TexturedPhysical::materialType = {
    "textured_physical",
    BGFX_INVALID_HANDLE,
    {
        { "baseColor", { bgfx::UniformType::Int1 } },
        { "normalMap", { bgfx::UniformType::Int1 } },
        { "occlusionRoughnessMetalness", { bgfx::UniformType::Int1 } },
    }
    };
}
}
