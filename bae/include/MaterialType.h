#pragma once
#include "utils/Shaders.h"
#include <bgfx/bgfx.h>
#include <string>
#include <utility>
#include <vector>

namespace bae {
// This class represents a shader program. Each instance is unique.
class MaterialType {
public:
    MaterialType() = default;
    MaterialType(
        const std::string& name);
    ~MaterialType();
    MaterialType(const MaterialType&) = delete;
    MaterialType& operator=(const MaterialType&) = delete;
    MaterialType(MaterialType&&) = delete;
    MaterialType& operator=(MaterialType&&) = delete;

    void init();
    void destroy();

    std::string name = "";
    bgfx::ProgramHandle program;
};
} // namespace bae
