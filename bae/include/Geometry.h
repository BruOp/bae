#pragma once
#include "./utils/Vertex.h"
#include <bgfx/bgfx.h>
#include <vector>

namespace bae
{
class Geometry
{
  public:
    Geometry() = default;
    Geometry(
        const std::vector<Vertex> &vertices,
        const std::vector<uint16_t> &indices) noexcept;

    void destroy() noexcept;
    void set(const bgfx::ViewId viewId) const;

  private:
    bgfx::VertexBufferHandle m_vertexBuffer;
    bgfx::IndexBufferHandle m_indexBuffer;
};
} // namespace bae
