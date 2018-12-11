#pragma once
#include "Geometry.h"
#include "MaterialType.h"

namespace bae {
template <class Material>
class Mesh {
public:
    Mesh() = default;
    Mesh(const Geometry& geometry, Material&& material)
        : geometry{ geometry }
        , material{ std::move(material) } {};

    // ~Mesh()
    // {
    //     material.destroy();
    // };

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = default;
    Mesh& operator=(Mesh&&) = default;

    inline void setup(const bgfx::ViewId viewId) const
    {
        // Set vertex and index buffer.
        geometry.set(viewId);
        material.setUniforms();
    }

    Geometry geometry;
    Material material;
};
} // namespace bae
