#include "utils/Shaders.h"

namespace bae
{
namespace Utils
{
const bgfx::Memory *loadMemory(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    if (size == -1) {
        throw std::runtime_error("Could not open file at " + filepath + " for reading.");
    }
    file.seekg(0, std::ios::beg);
    const bgfx::Memory *mem = bgfx::alloc(uint32_t(size + 1));
    if (file.read((char *)mem->data, size)) {
        mem->data[mem->size - 1] = '\0';
        return mem;
    }
    return nullptr;
}
}  // namespace Utils

namespace ShaderUtils
{
bgfx::ShaderHandle loadShader(const std::string& shaderName)
{
    std::string folder_path = SHADER_OUTPUT_DIR;

    switch (bgfx::getRendererType()) {
    case bgfx::RendererType::Noop:
    case bgfx::RendererType::Direct3D9:  folder_path += "/dx9/";   break;
    case bgfx::RendererType::Direct3D11:
    case bgfx::RendererType::Direct3D12: folder_path += "/dx11/";  break;
    case bgfx::RendererType::Gnm:        folder_path += "/pssl/";  break;
    case bgfx::RendererType::Metal:      folder_path += "/metal/"; break;
    case bgfx::RendererType::OpenGL:     folder_path += "/glsl/";  break;
    case bgfx::RendererType::OpenGLES:   folder_path += "/essl/";  break;
    case bgfx::RendererType::Vulkan:     folder_path += "/spirv/"; break;

    case bgfx::RendererType::Count:
        throw std::runtime_error("You should not be here!");
    }

    std::string full_path = folder_path + shaderName;
    full_path += ".bin";
    const bgfx::Memory *pShaderMemory = Utils::loadMemory(full_path);
    bgfx::ShaderHandle shaderHandle = bgfx::createShader(pShaderMemory);
    bgfx::setName(shaderHandle, shaderName.c_str());
    return shaderHandle;
}

bgfx::ProgramHandle loadProgram(
    const char* vertShaderName,
    const char* fragShaderName)
{
    bgfx::ShaderHandle vertShaderHandle = loadShader(vertShaderName);
    
    bgfx::ShaderHandle fragShaderHandle = BGFX_INVALID_HANDLE;
    if (fragShaderName != nullptr) {
        fragShaderHandle = loadShader(fragShaderName);
    }

    bgfx::ProgramHandle program = bgfx::createProgram(vertShaderHandle, fragShaderHandle, true);
    return program;
}

bgfx::ProgramHandle loadProgram(
    const std::string& shaderName)
{
    std::string vertShaderName = shaderName + "_vs";
    std::string fragShaderName = shaderName + "_fs";
    bgfx::ProgramHandle program = loadProgram(vertShaderName.c_str(), fragShaderName.c_str());
    return program;
}

}  // namespace ShaderUtils

}  // namespace bae
