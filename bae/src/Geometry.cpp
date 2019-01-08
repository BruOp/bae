#include "Geometry.h"

namespace bae {
void Geometry::set(const bgfx::ViewId viewId) const
{
    bgfx::setVertexBuffer(viewId, vertexBuffer);
    bgfx::setIndexBuffer(indexBuffer);
}

GeometryRegistry::~GeometryRegistry()
{
    for (auto& pair : geometry_map) {
        destroyGeometry(pair.second);
    }
}

void GeometryRegistry::create(
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
} // namespace bae
