#pragma once

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

namespace bae {
struct PosColorVertex {
    glm::vec3 pos;
    uint32_t color;
    // glm::vec2 texCoord;

    static bgfx::VertexDecl ms_declaration;

    static void init()
    {
        ms_declaration
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    };
};

struct PosTexNormalVertex {
    glm::vec3 pos;
    int16_t u = 0;
    int16_t v = 0;
    glm::vec3 normal;

    static bgfx::VertexDecl ms_declaration;

    static void init()
    {
        ms_declaration
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true)
            .end();
    };
};
} // namespace bae
