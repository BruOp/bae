#include "materials/Basic.h"

namespace bae {
namespace Materials {
    MaterialType basic = MaterialType{
        "basic",
        { { "color", { bgfx::UniformType::Vec4 } } }
    };

    const MaterialType* Basic::materialType = &basic;
}
}
