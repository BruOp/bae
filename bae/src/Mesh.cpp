#include "Mesh.h"

namespace bae
{
Mesh::Mesh()
    : m_geometry{}, m_program{}
{
}

Mesh::Mesh(const Geometry &geometry, const MaterialType &materialType)
    : m_geometry{geometry},
      m_program{materialType.m_program}
{
}

void Mesh::draw(const bgfx::ViewId viewId, const uint64_t state) const
{
    // Set vertex and index buffer.
    m_geometry.set(viewId);

    // Set render states.
    bgfx::setState(state);

    // Submit program
    bgfx::submit(viewId, m_program);
}
} // namespace bae
