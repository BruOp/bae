#include "materials/Lambertian.h"

namespace bae {
namespace Materials {
    MaterialType lambertian{};

    const MaterialType* Lambertian::materialType = &lambertian;
    const UniformInfoMap Lambertian::uniformInfoMap = {
        { "matColor", { bgfx::UniformType::Vec4 } },
    };
}
}
