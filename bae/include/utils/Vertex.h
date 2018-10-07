#pragma once

#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

namespace bae
{
struct Vertex
{
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
            // .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
            .end();
    };
};
} // namespace bae
