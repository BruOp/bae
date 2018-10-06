#include "Mesh.h"

namespace bae
{
Mesh::Mesh(const Geometry &geometry, const MaterialType &materialType)
    : m_pGeometry{&geometry},
      m_pMaterialType{&materialType}
{
}

void Mesh::draw(const bgfx::ViewId viewId, const uint64_t state) const
{
    // Set vertex and index buffer.
    m_pGeometry->set(viewId);

    // Set render states.
    bgfx::setState(state);

    // Submit program
    m_pMaterialType->submit(viewId);
}

} // namespace bae
