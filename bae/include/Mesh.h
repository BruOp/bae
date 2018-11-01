#pragma once
#include "Geometry.h"
#include "MaterialType.h"

namespace bae
{

class Mesh
{
  public:
    Mesh();
    Mesh(const Geometry &geometry, const MaterialType &materialType);

    void draw(const bgfx::ViewId viewId, const uint64_t state) const;

  private:
    Geometry m_geometry;
    bgfx::ProgramHandle m_program;
};
} // namespace bae
