#include "MaterialType.h"
#include <iostream>

namespace bae {
MaterialTypeManager::~MaterialTypeManager() noexcept
{
    for (auto& materialType : materialTypes) {
        materialType.destroy();
    }
}

void MaterialType::destroy() noexcept
{
    for (auto& entry : uniformInfoMap) {
        bgfx::UniformHandle handle = entry.second.handle;
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    }

    if (bgfx::isValid(program)) {
        bgfx::destroy(program);
    }
}
} // namespace bae
