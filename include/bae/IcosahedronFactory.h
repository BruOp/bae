#pragma once
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

namespace bae {

    struct Mesh;

    struct BasicVertex
    {
        glm::vec3 position;

        static bgfx::VertexDecl s_decl;
        static void init()
        {
            s_decl.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .end();
        }
    };

    extern std::vector<BasicVertex> basicIcosahedronPositions;

    class IcosahedronFactory
    {
    public:
        IcosahedronFactory(uint8_t detail);

        Mesh getMesh();

        std::vector<BasicVertex> vertices;
        std::vector<uint16_t> indices;


    private:
        uint8_t detail;
        std::unordered_map<uint32_t, uint16_t> newVertices;

        uint16_t getOrCreateMidPoint(uint16_t first, uint16_t second);
    };
}
