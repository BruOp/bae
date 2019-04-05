#pragma once
#include <bgfx/bgfx.h>

namespace bae
{

    struct ScreenSpaceQuadVertex
    {
        float m_x;
        float m_y;
        float m_z;
        uint32_t m_rgba;
        float m_u;
        float m_v;

        static void init()
        {
            ms_decl
                .begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .end();
        }

        static bgfx::VertexDecl ms_decl;
    };

    // Ripped out of BGFX examples
    void setScreenSpaceQuad(
        const float _textureWidth,
        const float _textureHeight,
        const bool _originBottomLeft = false,
        const float _width = 1.0f,
        const float _height = 1.0f);

}