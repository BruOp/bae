#include "utils/Vertex.h"

namespace bae {
bgfx::VertexDecl PosVertex::ms_declaration;
bgfx::VertexDecl NormalVertex::ms_declaration;
bgfx::VertexDecl TangentVertex::ms_declaration;
bgfx::VertexDecl TexCoordVertex::ms_declaration;
bgfx::VertexDecl PosColorVertex::ms_declaration;
bgfx::VertexDecl PosTexNormalVertex::ms_declaration;
}
