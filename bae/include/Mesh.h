#pragma once
#include "Geometry.h"
#include "MaterialType.h"

namespace bae
{

class Mesh
{
  public:
    Mesh() = default;
    Mesh(const Geometry &geometry, const MaterialType &materialType);

    void draw(const bgfx::ViewId viewId, const uint64_t state) const;

  private:
    Geometry const *m_pGeometry = nullptr;
    MaterialType const *m_pMaterialType = nullptr;
};
} // namespace bae
