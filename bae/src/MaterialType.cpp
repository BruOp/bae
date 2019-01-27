#include "MaterialType.h"
#include <iostream>

namespace bae {
MaterialType MaterialTypeManager::createMaterialType(
    const std::string& name,
    const UniformInfoMap& uniformHandleMap) noexcept
{
    materialTypes[name] = MaterialType{
        ShaderUtils::loadProgram(name),
        uniformHandleMap
    };
    MaterialType& matType = materialTypes[name];

    for (auto& entry : matType.uniformHandleMap) {
        const std::string& name = entry.first;
        UniformHandleInfo& info = entry.second;
        info.handle = bgfx::createUniform(name.c_str(), info.type);
    }

    return materialTypes[name];
}

MaterialTypeManager::~MaterialTypeManager() noexcept
{
    for (auto& materialTypeEntry : materialTypes) {
        materialTypeEntry.second.destroy();
    }
}

void MaterialType::destroy() noexcept
{
    for (auto& entry : uniformHandleMap) {
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
