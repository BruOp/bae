#include "Geometry.h"

namespace bae
{
Geometry::Geometry(
    const std::vector<Vertex> &vertices,
    const std::vector<uint16_t> &indices) noexcept
{
    const bgfx::Memory *vertMemory = bgfx::copy(vertices.data(), sizeof(vertices[0]) * vertices.size());
    m_vertexBuffer = bgfx::createVertexBuffer(
        vertMemory,
        Vertex::ms_declaration);

    const bgfx::Memory *indexMemory = bgfx::copy(indices.data(), sizeof(uint16_t) * indices.size());
    m_indexBuffer = bgfx::createIndexBuffer(indexMemory);
}

Geometry::~Geometry()
{
    bgfx::destroy(m_indexBuffer);
    bgfx::destroy(m_vertexBuffer);
}

void Geometry::set(const bgfx::ViewId viewId) const
{
    bgfx::setVertexBuffer(viewId, m_vertexBuffer);
    bgfx::setIndexBuffer(m_indexBuffer);
}
} // namespace bae
