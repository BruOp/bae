#include "MaterialType.h"

namespace bae
{
MaterialType::MaterialType(
    const std::string &name,
    const std::string &vertShaderName,
    const std::string &fragShaderName)
    : name{name},
      m_program{ShaderUtils::loadProgram(vertShaderName, vertShaderName)}
{
}

MaterialType::~MaterialType()
{
    bgfx::destroy(m_program);
}

void MaterialType::submit(const bgfx::ViewId viewId) const
{
    bgfx::submit(viewId, m_program);
}
} // namespace bae
