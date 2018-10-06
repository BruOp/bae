#pragma once
#include "utils/Shaders.h"
#include <bgfx/bgfx.h>
#include <string>

namespace bae
{
// This class represents a shader program. Each instance is unique.
// Eventually it will need to support a uniform specificificaiton of some
// sort.
class MaterialType
{
  public:
    MaterialType(
        const std::string &name,
        const std::string &vertShaderName,
        const std::string &fragShaderName);
    ~MaterialType();
    MaterialType(const MaterialType &) = delete;
    MaterialType &operator=(const MaterialType &) = delete;
    MaterialType(MaterialType &&) = delete;
    MaterialType &operator=(MaterialType &&) = delete;

    void submit(const bgfx::ViewId viewId) const;

  private:
    std::string name;
    bgfx::ProgramHandle m_program;
};
} // namespace bae
