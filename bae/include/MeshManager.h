#pragma once

#include <memory>
#include <vector>

#include "Mesh.h"

namespace bae {

template <class Material>
class MeshManager {
public:
    MeshManager() = default;

    MeshManager(const MeshManager&) = delete;
    MeshManager& operator=(const MeshManager) = delete;

    MeshManager(MeshManager&&) = default;
    MeshManager& operator=(MeshManager&&) = default;

    void addMesh(Mesh<Material>&& mesh)
    {
        meshes.push_back(std::move(mesh));
    };

    void addMesh(const Geometry& geom, Material&& material)
    {
        addMesh(Mesh<Material>{ geom, std::move(material) });
    };

    void draw(const bgfx::ViewId viewId, const uint64_t state) const
    {
        bgfx::ProgramHandle program = Material::materialType->program;
        for (auto& mesh : meshes) {
            mesh.setup(viewId);
            bgfx::setState(state);
            bgfx::submit(viewId, program);
        }
    };

private:
    std::vector<Mesh<Material>> meshes;
};
} // namespace bae
