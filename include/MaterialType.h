#pragma once
#include <bgfx/bgfx.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "utils/Shaders.h"

namespace bae
{
struct UniformHandleInfo {
    bgfx::UniformType::Enum type;
    bgfx::UniformHandle handle = BGFX_INVALID_HANDLE;
};

typedef std::unordered_map<std::string, UniformHandleInfo> UniformInfoMap;

struct MaterialType {
    std::string name;
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    UniformInfoMap uniformInfoMap;

    inline bgfx::UniformHandle getHandle(const std::string& uniformName) const
    {
        return uniformInfoMap.at(uniformName).handle;
    }

    void destroy() noexcept;
};

class MaterialTypeManager
{
   public:
    MaterialTypeManager() = default;
    ~MaterialTypeManager();
    MaterialTypeManager(const MaterialTypeManager&) = delete;
    MaterialTypeManager& operator=(const MaterialTypeManager&) = delete;
    MaterialTypeManager(MaterialTypeManager&&) = default;
    MaterialTypeManager& operator=(MaterialTypeManager&&) = default;

    template <typename Material>
    void registerMaterialType() noexcept
    {
        MaterialType& matType = Material::materialType;
        matType.program = ShaderUtils::loadProgram(matType.name);

        for (auto& entry : matType.uniformInfoMap) {
            const std::string& name = entry.first;
            UniformHandleInfo& info = entry.second;
            info.handle = bgfx::createUniform(name.c_str(), info.type);
        }

        materialTypes.push_back(matType);
    };

   private:
    std::vector<MaterialType> materialTypes;
};
}  // namespace bae
