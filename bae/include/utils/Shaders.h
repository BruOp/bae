#pragma once
#include <bgfx/bgfx.h>
#include <fstream>
#include <string>

namespace bae
{
namespace Utils
{
const bgfx::Memory *loadMemory(const std::string &filepath);
} // namespace Utils

namespace ShaderUtils
{
bgfx::ShaderHandle loadShader(const std::string &shaderName);

bgfx::ProgramHandle loadProgram(
    const std::string &vertShaderName,
    const std::string &fragShaderName);
bgfx::ProgramHandle loadProgram(
    const std::string &shaderName);
} // namespace ShaderUtils
} // namespace bae
