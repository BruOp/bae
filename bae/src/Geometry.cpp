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
} // namespace bae
