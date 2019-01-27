#include "materials/Basic.h"

namespace bae {
namespace Materials {
    MaterialType basic{};

    const MaterialType* Basic::materialType = &basic;
    const UniformInfoMap Basic::uniformInfoMap = {
        { "color", { bgfx::UniformType::Vec4 } },
    };
}
}
