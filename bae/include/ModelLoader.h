#pragma once
#include "tiny_obj_loader.h"
#include <exception>

#include "Geometry.h"
#include "utils/Vertex.h"

namespace bae {

class ModelLoader {
public:
    ModelLoader() = default;
    ModelLoader(GeometryRegistry* geoRegistry)
        : geometryRegistry{ geoRegistry } {};

    Geometry loadModel(const std::string& name, const std::string& file)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str(), nullptr, true)) {
            throw std::runtime_error(err);
        }

        std::vector<PosTexNormalVertex> vertices{};
        std::vector<uint16_t> indices{};

        std::unordered_map<PosTexNormalVertex, uint32_t> uniqueVertices = {};

        auto& shape = shapes[0];
        for (const auto& index : shape.mesh.indices) {

            PosTexNormalVertex vertex{
                { attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2] },
                0,
                0,
                { attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2] },
            };
            if (index.texcoord_index >= 0) {
                vertex.u = attrib.normals[3 * index.texcoord_index];
                vertex.v = attrib.normals[3 * index.texcoord_index + 1];
            }

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }

        geometryRegistry->create(name, vertices, indices);
        return geometryRegistry->get(name);
    }

private:
    GeometryRegistry* geometryRegistry = nullptr;
};
}
