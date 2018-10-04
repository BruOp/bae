#include <bgfx/bgfx.h>
#include <glm/include/GLFW/glfw3.h>

namespace bae {
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        uint32_t _abgr;

        static void init()
        {
            s_declaration
                .begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
                .end();
        };

        static bgfx::VertexDecl s_declaration;
    };
}
