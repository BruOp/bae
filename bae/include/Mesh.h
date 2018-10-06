#pragma once
#include "Geometry.h"
#include "MaterialType.h"

namespace bae
{

class Mesh
{
  public:
    Mesh(const Geometry &geometry, const MaterialType &materialType);

    void draw(const bgfx::ViewId viewId, const uint64_t state) const;

  private:
    Geometry const *m_pGeometry;
    MaterialType const *m_pMaterialType;
};
} // namespace bae
