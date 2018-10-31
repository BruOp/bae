#include "utils/Shaders.h"

namespace bae
{
namespace Utils
{
const bgfx::Memory *loadMemory(const std::string &filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    if (size == -1)
    {
        throw std::runtime_error("Could not open file at " + filepath + " for reading.");
    }
    file.seekg(0, std::ios::beg);
    const bgfx::Memory *mem = bgfx::alloc(uint32_t(size + 1));
    if (file.read((char *)mem->data, size))
    {
        mem->data[mem->size - 1] = '\0';
        return mem;
    }
    return nullptr;
}
} // namespace Utils

namespace ShaderUtils
{
bgfx::ShaderHandle loadShader(const std::string &shaderName)
{
    std::string folder_path = SHADER_OUTPUT_DIR;
    folder_path += "/glsl/";
    std::string full_path = folder_path + shaderName;
    const bgfx::Memory *pShaderMemory = Utils::loadMemory(full_path);
    bgfx::ShaderHandle shaderHandle = bgfx::createShader(pShaderMemory);
    return shaderHandle;
}

bgfx::ProgramHandle loadProgram(
    const std::string &vertShaderName,
    const std::string &fragShaderName)
{
    auto vertShaderHandle = loadShader(vertShaderName);
    auto fragShaderHandle = loadShader(fragShaderName);
    auto program = bgfx::createProgram(vertShaderHandle, fragShaderHandle, true);
    return program;
}

bgfx::ProgramHandle loadProgram(
    const std::string &shaderName)
{
    std::string vertShaderName = shaderName + "_vs.bin";
    std::string fragShaderName = shaderName + "_fs.bin";
    auto program = loadProgram(vertShaderName, fragShaderName);
    return program;
}

} // namespace ShaderUtils

} // namespace bae
