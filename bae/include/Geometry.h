#pragma once
#include "./utils/Vertex.h"
#include <bgfx/bgfx.h>
#include <unordered_map>
#include <vector>

namespace bae {
struct Geometry {
    bgfx::VertexBufferHandle vertexBuffer = { bgfx::kInvalidHandle };
    bgfx::IndexBufferHandle indexBuffer = { bgfx::kInvalidHandle };

    void set(const bgfx::ViewId viewId) const;
};

class GeometryRegistry {
public:
    ~GeometryRegistry();

    inline Geometry get(const std::string& name) const
    {
        return geometry_map.at(name);
    }

    template <typename Vertex>
    void create(
        const std::string& name,
        const std::vector<Vertex>& vertices,
        const std::vector<uint16_t>& indices) noexcept
    {
        Geometry geometry{};
        const bgfx::Memory* vertMemory = bgfx::copy(vertices.data(), sizeof(vertices[0]) * vertices.size());
        geometry.vertexBuffer = bgfx::createVertexBuffer(
            vertMemory,
            Vertex::ms_declaration);

        const bgfx::Memory* indexMemory = bgfx::copy(indices.data(), sizeof(indices[0]) * indices.size());
        geometry.indexBuffer = bgfx::createIndexBuffer(indexMemory);
        // Push it into our map
        geometry_map[name] = geometry;
    };

private:
    std::unordered_map<std::string, Geometry> geometry_map;

    inline static void destroyGeometry(Geometry& geometry) noexcept
    {
        bgfx::destroy(geometry.indexBuffer);
        bgfx::destroy(geometry.vertexBuffer);
    };
};
} // namespace bae
