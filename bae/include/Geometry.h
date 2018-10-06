#pragma once
#include "./utils/Vertex.h"
#include <bgfx/bgfx.h>
#include <vector>

namespace bae
{
class Geometry
{
  public:
    Geometry(
        const std::vector<Vertex> &vertices,
        const std::vector<uint16_t> &indices) noexcept;
    ~Geometry() noexcept;
    Geometry(const Geometry &) = delete;
    Geometry &operator=(const Geometry &) = delete;
    Geometry(Geometry &&) = default;
    Geometry &operator=(Geometry &&) = default;

    void set(const bgfx::ViewId viewId) const;

  private:
    bgfx::VertexBufferHandle m_vertexBuffer;
    bgfx::IndexBufferHandle m_indexBuffer;
};
} // namespace bae
