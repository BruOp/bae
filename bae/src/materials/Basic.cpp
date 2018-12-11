#include "materials/Basic.h"

namespace bae {
namespace Materials {
    MaterialType basic = MaterialType{ "basic" };

    const MaterialType* Basic::materialType = &basic;
}
}
