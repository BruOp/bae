#include "MaterialType.h"
#include <iostream>

namespace bae {
// ----------- MATERIALS -----------

MaterialType::MaterialType(const std::string& name)
    : name{ name }
    , program{}
{
}

MaterialType::~MaterialType()
{
}

void MaterialType::init()
{
    program = ShaderUtils::loadProgram(name);
}

void MaterialType::destroy()
{
    std::cout << "Destroying Program " << program.idx << std::endl;
    bgfx::destroy(program);
}
} // namespace bae
