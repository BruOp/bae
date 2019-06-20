#include "IcosahedronFactory.h"
#include <bx/math.h>
#include <PhysicallyBasedScene.h>

namespace bae {
    bgfx::VertexDecl BasicVertex::s_decl;

    float t = (1.0 + bx::sqrt(5.0)) / 2.0;

    std::vector<BasicVertex> basicIcosahedronPositions = {
        {{-1.0,  t,  0.0}},
        {{ 1.0,  t,  0.0}},
        {{-1.0, -t,  0.0}},
        {{ 1.0, -t,  0.0}},

        {{ 0.0, -1.0,  t}},
        {{ 0.0,  1.0,  t}},
        {{ 0.0, -1.0, -t}},
        {{ 0.0,  1.0, -t}},

        {{ t, 0.0, -1.0}},
        {{ t, 0.0,  1.0}},
        {{-t, 0.0, -1.0}},
        {{-t, 0.0,  1.0}},
    };

    std::vector<uint16_t> basicIcosahedronIndices = {
        0, 11, 5,
        0, 5, 1,
        0, 1, 7,
        0, 7, 10,
        0, 10, 11,
        1, 5, 9,
        5, 11, 4,
        11, 10, 2,
        10, 7, 6,
        7, 1, 8,
        3, 9, 4,
        3, 4, 2,
        3, 2, 6,
        3, 6, 8,
        3, 8, 9,
        4, 9, 5,
        2, 4, 11,
        6, 2, 10,
        8, 6, 7,
        9, 8, 1
    };

    IcosahedronFactory::IcosahedronFactory(uint8_t detail)
        : detail{ detail },
        vertices{ basicIcosahedronPositions },
        indices{ basicIcosahedronIndices }
    {
        for (auto& v : vertices) {
            v.position = glm::normalize(v.position);
        }

        for (uint8_t subdivisionLevel = 0; subdivisionLevel < detail; ++subdivisionLevel) {
            size_t numIndices = indices.size();
            std::vector<uint16_t> newIndices(numIndices * 4);
            // For each face
            for (size_t i = 0; i < numIndices; i += 3)
            {
                uint16_t i_a = indices[i];
                uint16_t i_b = indices[i + 1];
                uint16_t i_c = indices[i + 2];

                uint16_t i_ab = getOrCreateMidPoint(i_a, i_b);
                uint16_t i_bc = getOrCreateMidPoint(i_b, i_c);
                uint16_t i_ac = getOrCreateMidPoint(i_a, i_c);

                newIndices.insert(newIndices.end(), {
                    i_a, i_ab, i_ac,
                    i_b, i_bc, i_ab,
                    i_c, i_ac, i_bc,
                    i_ab, i_bc, i_ac
                });
            }
            indices = std::move(newIndices);
        }
    }

    Mesh IcosahedronFactory::getMesh()
    {
        const bgfx::Memory* vertMemory = bgfx::copy(vertices.data(), uint32_t(vertices.size()) * sizeof(vertices[0]));
        const bgfx::Memory* indexMemory = bgfx::copy(indices.data(), uint32_t(indices.size()) * sizeof(indices[0]));

        Mesh mesh{
            bgfx::createVertexBuffer(vertMemory, BasicVertex::s_decl),
            bgfx::createIndexBuffer(indexMemory),
        };

        return mesh;
    }

    uint16_t IcosahedronFactory::getOrCreateMidPoint(uint16_t first, uint16_t second)
    {
        uint32_t smaller = bx::min(first, second);
        uint32_t larger = bx::max(first, second);
        uint32_t key = (smaller << 16) | larger;
        auto& iter = newVertices.find(key);

        if (iter != newVertices.end()) {
            return iter->second;
        }
        // Calculate and store new vertex
        glm::vec3 firstPos = vertices[first].position;
        glm::vec3 secondPos = vertices[second].position;

        BasicVertex midPoint{ glm::normalize(glm::vec3{ 0.5f * (secondPos + firstPos) }) };
        uint16_t newIndex = uint16_t(vertices.size());
        vertices.push_back(midPoint);
        newVertices[key] = newIndex;
        return newIndex;
    };
}
