#include "MaterialType.h"
#include <iostream>

namespace bae
{
// ----------- MATERIALS -----------

MaterialType::MaterialType(const std::string &name)
    : m_name{name},
      m_program{}
{
}

MaterialType::~MaterialType()
{
}

void MaterialType::init()
{
    m_program = ShaderUtils::loadProgram(m_name);
}

void MaterialType::destroy()
{
    std::cout << "Destroying Program " << m_program.idx << std::endl;
    bgfx::destroy(m_program);
}

void MaterialType::submit(const bgfx::ViewId viewId) const
{
    bgfx::submit(viewId, m_program);
}

// ----------- UNIFORM SETS -----------

UniformSet::UniformSet(const UniformSetPrototype &uniformSetProtype)
    : m_proto{&uniformSetProtype},
      m_uniforms{}
{
    m_uniforms.resize(uniformSetProtype.size());
    for (const auto &prototype : uniformSetProtype.m_prototypes)
    {
        m_uniforms.push_back(bgfx::createUniform(prototype.name.data(), prototype.uniformType));
    }
}

UniformSet::~UniformSet()
{
    std::cout << "Destroying Uniforms \n";
    for (auto &uniform : m_uniforms)
    {
        std::cout << "Destroying Uniform " << uniform.idx << std::endl;
        bgfx::destroy(uniform);
    }
}

UniformSet::UniformSet(UniformSet &&otherSet)
    : m_proto{otherSet.m_proto},
      m_uniforms{std::move(otherSet.m_uniforms)}
{
    otherSet.m_proto = nullptr;
}

UniformSet &UniformSet::operator=(UniformSet &otherSet)
{
    if (this != &otherSet)
    {
        m_proto = otherSet.m_proto;
        m_uniforms = std::move(otherSet.m_uniforms);

        otherSet.m_proto = nullptr;
    }
    return *this;
}

UniformSet &&UniformSet::copy(const UniformSet &uniformSet) const
{
    auto set = UniformSet(*uniformSet.m_proto);
    return std::move(set);
}
} // namespace bae
