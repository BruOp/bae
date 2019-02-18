#pragma once
#include "./utils/Vertex.h"
#include <bgfx/bgfx.h>
#include <unordered_map>
#include <vector>
#include <array>


namespace bae {
	constexpr size_t MAX_VERT_BUFFERS_PER_GEOMETRY = 4;

struct Geometry {
	bgfx::VertexBufferHandle vertexBuffers[MAX_VERT_BUFFERS_PER_GEOMETRY] = { BGFX_INVALID_HANDLE, BGFX_INVALID_HANDLE, BGFX_INVALID_HANDLE, BGFX_INVALID_HANDLE };
    bgfx::IndexBufferHandle indexBuffer = BGFX_INVALID_HANDLE;
	uint16_t numVertBufferStreams = 0;

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
		geometry.numVertBufferStreams = 1;
        geometry.vertexBuffers[0] = bgfx::createVertexBuffer(
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
		for (uint16_t i = 0; i < geometry.numVertBufferStreams; ++i) {
			bgfx::destroy(geometry.vertexBuffers[i]);
		}
    };
};
} // namespace bae
