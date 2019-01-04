#pragma once
#include "utils/Shaders.h"
#include <bgfx/bgfx.h>
#include <string>
#include <unordered_map>
#include <utility>

namespace bae {
struct UniformHandleInfo {
    bgfx::UniformType::Enum type;
    bgfx::UniformHandle handle = { bgfx::kInvalidHandle };
};

// This class represents a shader program. Each instance is unique.
class MaterialType {
public:
    MaterialType(
        const std::string& name,
        const std::unordered_map<std::string, UniformHandleInfo>& uniformHandleMap);
    ~MaterialType() = default;
    MaterialType(const MaterialType&) = delete;
    MaterialType& operator=(const MaterialType&) = delete;
    MaterialType(MaterialType&&) = delete;
    MaterialType& operator=(MaterialType&&) = delete;

    void init() noexcept;
    void destroy() noexcept;

    inline bgfx::ProgramHandle getProgram() const
    {
        return program;
    };

    inline bgfx::UniformHandle getHandle(const std::string& name) const
    {
        return uniformHandleMap.at(name).handle;
    }

private:
    std::string name;
    bgfx::ProgramHandle program;
    std::unordered_map<std::string, UniformHandleInfo> uniformHandleMap;
};
} // namespace bae
