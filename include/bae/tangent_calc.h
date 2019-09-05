#pragma once
#include "mikktspace/mikktspace.h"
#include "PhysicallyBasedScene.h"

namespace bae
{
    struct VertexData
    {
        uint16_t* p_indices;
        // Different handle for each "stream" of vertex attributes
        // 0 - Position
        // 1 - Normal
        // 2 - Tangent
        // 3 - TexCoord0
        unsigned char* data[4];
        size_t byteLengths[4];
        size_t numFaces;
        size_t numVertices;
    };

    namespace MikktSpace
    {
        int getNumFaces(const SMikkTSpaceContext* ctx)
        {
            const VertexData* vertexData = static_cast<const VertexData*>(ctx->m_pUserData);
            return vertexData->numFaces;
        };

        int getNumVerticesOfFace(const SMikkTSpaceContext*, const int)
        {
            return 3;
        };

        void getNormal(const SMikkTSpaceContext* ctx, float normals[], const int iface, const int ivert)
        {
            int i = iface * 3 + ivert;
            const VertexData* vertexData = static_cast<const VertexData*>(ctx->m_pUserData);
            uint16_t index = *(vertexData->p_indices + i);
            memcpy(normals, vertexData->data[1] + index * sizeof(glm::vec3), sizeof(glm::vec3));
        };

        void getTexCoord(const SMikkTSpaceContext* ctx, float texCoordOut[], const int iface, const int ivert)
        {
            int i = iface * 3 + ivert;
            const VertexData* vertexData = static_cast<const VertexData*>(ctx->m_pUserData);
            uint16_t index = *(vertexData->p_indices + i);
            memcpy(texCoordOut, vertexData->data[3] + index * sizeof(glm::vec2), sizeof(glm::vec2));
        };

        void getPosition(const SMikkTSpaceContext* ctx, float positions[], const int iface, const int ivert)
        {

            int i = iface * 3 + ivert;
            const VertexData* vertexData = static_cast<const VertexData*>(ctx->m_pUserData);
            uint16_t index = *(vertexData->p_indices + i);
            memcpy(positions, vertexData->data[0] + index * sizeof(glm::vec3), sizeof(glm::vec3));
        };

        void setTSpaceBasic(const SMikkTSpaceContext* ctx, const float tangent[], const float sign,
            const int iface, const int ivert)
        {
            int i = iface * 3 + ivert;
            const VertexData* vertexData = static_cast<const VertexData*>(ctx->m_pUserData);
            uint16_t index = *(vertexData->p_indices + i);
            // Invert the sign because of glTF convention.
            glm::vec4 t{ tangent[0], tangent[1], tangent[2], -sign };
            memcpy(vertexData->data[2] + index * sizeof(glm::vec4), &(t.x), sizeof(glm::vec4));
        };


        void calcTangents(const VertexData& vertexData)
        {
            SMikkTSpaceInterface iface;
            iface.m_getNumFaces = getNumFaces;
            iface.m_getNumVerticesOfFace = getNumVerticesOfFace;
            iface.m_getPosition = getPosition;
            iface.m_getNormal = getNormal;
            iface.m_getTexCoord = getTexCoord;
            iface.m_setTSpaceBasic = setTSpaceBasic;
            iface.m_setTSpace = nullptr;

            SMikkTSpaceContext context{
                &iface,
                (void*)& vertexData,
            };
            genTangSpaceDefault(&context);
        }
    }
}