#pragma once
#include <bgfx/bgfx.h>
#include <functional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace bae
{
struct PosColorVertex {
    glm::vec3 pos;
    uint32_t color;

    static bgfx::VertexDecl ms_declaration;

    static void init()
    {
        ms_declaration.begin()
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
        ms_declaration.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true)
            .end();
    };
    bool operator==(const PosTexNormalVertex& other) const
    {
        return pos == other.pos && u == other.u && v == other.v && normal == other.normal;
    }
};

inline void initializeVertexDecls()
{
    PosColorVertex::init();
    PosTexNormalVertex::init();
}
}  // namespace bae

namespace std
{
template <>
struct hash<bae::PosTexNormalVertex> {
    size_t operator()(const bae::PosTexNormalVertex& vertex) const
    {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<int16_t>()(vertex.u) << 1)) ^
                (hash<int16_t>()(vertex.v) << 1) >> 1) ^
               (hash<glm::vec3>()(vertex.normal) << 1);
    }
};
}  // namespace std
