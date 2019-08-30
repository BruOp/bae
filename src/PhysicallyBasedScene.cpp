#include "PhysicallyBasedScene.h"
#include <stdexcept>
#include <iostream>

#include "bgfx_utils.h"

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

namespace bae
{
    void Vertex::init()
    {
        ms_decl.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true)
            .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float, true)
            .add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float, true)
            .end();
    }

    bgfx::VertexDecl Vertex::ms_decl;


    void destroy(const Mesh& mesh)
    {
        for (const bgfx::VertexBufferHandle vertexHandle : mesh.vertexHandles) {
            bgfx::destroy(vertexHandle);
        }
        bgfx::destroy(mesh.indexHandle);
    }


    void destroy(Model& model)
    {
        model.opaqueMeshes.destroy();
        model.maskedMeshes.destroy();
        model.transparentMeshes.destroy();

        for (const bgfx::TextureHandle texture : model.textures) {
            bgfx::destroy(texture);
        }
    }
}