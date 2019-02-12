#pragma once
#include "utils/Shaders.h"
#include <bgfx/bgfx.h>
#include <string>
#include <unordered_map>
#include <utility>

namespace bae {
struct UniformHandleInfo {
    bgfx::UniformType::Enum type;
    bgfx::UniformHandle handle = BGFX_INVALID_HANDLE;
};

typedef std::unordered_map<std::string, UniformHandleInfo> UniformInfoMap;

struct MaterialType {
    std::string name;
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    UniformInfoMap uniformHandleMap;

    inline bgfx::UniformHandle getHandle(const std::string& uniformName) const
    {
        return uniformHandleMap.at(uniformName).handle;
    }

    void destroy() noexcept;
};

class MaterialTypeManager {

public:
    MaterialTypeManager() = default;
    ~MaterialTypeManager();
    MaterialTypeManager(const MaterialTypeManager&) = delete;
    MaterialTypeManager& operator=(const MaterialTypeManager&) = delete;
    MaterialTypeManager(MaterialTypeManager&&) = default;
    MaterialTypeManager& operator=(MaterialTypeManager&&) = default;

    MaterialType createMaterialType(
        const std::string& name,
        const UniformInfoMap& uniformHandleMap) noexcept;

private:
    std::unordered_map<std::string, MaterialType> materialTypes;
};
} // namespace bae
