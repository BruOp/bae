#include "MaterialType.h"
#include <iostream>

namespace bae {
// ----------- MATERIALS -----------

MaterialType::MaterialType(
    const std::string& name,
    const std::unordered_map<std::string, UniformHandleInfo>& uniformHandleMap)
    : name{ name }
    , program{}
    , uniformHandleMap{ uniformHandleMap }
{
}

void MaterialType::init() noexcept
{
    program = ShaderUtils::loadProgram(name);

    for (auto& entry : uniformHandleMap) {
        const std::string& name = entry.first;
        UniformHandleInfo& info = entry.second;
        info.handle = bgfx::createUniform(name.c_str(), info.type);
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

    std::cout << "Destroying Program " << program.idx << std::endl;
    bgfx::destroy(program);
}
} // namespace bae
