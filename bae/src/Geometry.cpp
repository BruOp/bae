#include "Geometry.h"

namespace bae {
void Geometry::set(const bgfx::ViewId viewId) const
{
	for (uint16_t i = 0; i < numVertBufferStreams; ++i) {
		bgfx::setVertexBuffer(i, vertexBuffers[i]);
	}
    bgfx::setIndexBuffer(indexBuffer);
}

GeometryRegistry::~GeometryRegistry()
{
    for (auto& pair : geometry_map) {
        destroyGeometry(pair.second);
    }
}
} // namespace bae
