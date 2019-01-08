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

    void create(
        const std::string& name,
        const std::vector<Vertex>& vertices,
        const std::vector<uint16_t>& indices) noexcept;

private:
    std::unordered_map<std::string, Geometry> geometry_map;

    inline static void destroyGeometry(Geometry& geometry) noexcept
    {
        bgfx::destroy(geometry.indexBuffer);
        bgfx::destroy(geometry.vertexBuffer);
    };
};
} // namespace bae
