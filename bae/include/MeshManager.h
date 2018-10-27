#pragma once

#include <memory>
#include <vector>

#include "Mesh.h"

namespace bae
{

class MeshManager
{
  public:
    MeshManager()
    {
        m_meshes.resize(128);
    };
    ~MeshManager()
    {
    }

    void addMesh(const Geometry &geom, const MaterialType &material);

  private:
    typedef std::unique_ptr<Mesh> UniqMesh;

    std::vector<UniqMesh> m_meshes;
};
} // namespace bae
