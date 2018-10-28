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

// Trying to specify what's required for the material, in terms of uniforms. So maybe
// this is just a list of Uniforms... But does it make sense of this to handle the creation of the UniformHandles or should that happen as part of the material?
// 
class MaterialDescriptorSet
{
};
} // namespace bae
