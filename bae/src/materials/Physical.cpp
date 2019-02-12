#include "materials/Physical.h"

namespace bae {
namespace Materials {
    MaterialType physical{};

    const MaterialType* Physical::materialType = &physical;
    const UniformInfoMap Physical::uniformInfoMap = {
        { "matColor", { bgfx::UniformType::Vec4 } },
        { "metallicRoughnessReflectance", { bgfx::UniformType::Vec4 } },
    };
}
}
