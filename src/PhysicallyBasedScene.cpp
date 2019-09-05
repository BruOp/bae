#include "PhysicallyBasedScene.h"

namespace bae
{
    void destroy(const Mesh& mesh)
    {
        for (uint8_t i = 0; i < mesh.numVertexHandles; ++i) {
            bgfx::destroy(mesh.vertexHandles[i]);
        }
        bgfx::destroy(mesh.indexHandle);
    }


    void destroy(Model& model)
    {
        for (Mesh& mesh : model.opaqueMeshes.meshes) {
            destroy(mesh);
        }
        for (Mesh& mesh : model.maskedMeshes.meshes) {
            destroy(mesh);
        }
        for (Mesh& mesh : model.transparentMeshes.meshes) {
            destroy(mesh);
        }

        for (const bgfx::TextureHandle texture : model.textures) {
            bgfx::destroy(texture);
        }
    }
}