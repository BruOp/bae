#pragma once
#include "utils/Shaders.h"
#include <bgfx/bgfx.h>
#include <string>
#include <utility>
#include <vector>

namespace bae
{
// This class represents a shader program. Each instance is unique.
// Eventually it will need to support a uniform specificificaiton of some
// sort.
class MaterialType
{
  public:
    MaterialType() = default;
    MaterialType(
        const std::string &name);
    ~MaterialType();
    MaterialType(const MaterialType &) = delete;
    MaterialType &operator=(const MaterialType &) = delete;
    MaterialType(MaterialType &&) = delete;
    MaterialType &operator=(MaterialType &&) = delete;

    void init();
    void destroy();

    std::string m_name = "";
    bgfx::ProgramHandle m_program;
};

struct UniformPrototype
{
    std::string name;
    bgfx::UniformType::Enum uniformType;
    uint16_t num;
};

struct UniformSetPrototype
{
    std::vector<UniformPrototype> m_prototypes;

    inline size_t size() const { return m_prototypes.size(); }
};

// Handles the creation, setting, and destruction of BGFX uniform handles
class UniformSet
{
  public:
    UniformSet() = default;
    UniformSet(const UniformSetPrototype &uniformSetProtype);
    ~UniformSet();

    UniformSet(const UniformSet &otherSet) = delete;
    UniformSet &operator=(const UniformSet &otherSet) = delete;

    UniformSet(UniformSet &&otherSet);
    UniformSet &operator=(UniformSet &otherSet);

    void setUniform(size_t index, const void *data)
    {
        bgfx::setUniform(m_uniforms[index], data);
    };

    UniformSet &&copy(const UniformSet &uniformSet) const;

  private:
    const UniformSetPrototype *m_proto = nullptr;
    std::vector<bgfx::UniformHandle> m_uniforms = {};
};
} // namespace bae
