#pragma once
#include <exception>

#include "Geometry.h"
#include "utils/Vertex.h"

namespace tinygltf {
	class Node;
	class Model;
	class Mesh;
}

namespace bae {

class ModelLoader {
public:
    ModelLoader() = default;
    ModelLoader(GeometryRegistry* geoRegistry)
        : pGeometryRegistry{ geoRegistry } {};

	Geometry loadObjModel(const std::string& name, const std::string& file) noexcept;

	std::vector<Geometry> loadGltfModel(const std::string& file) noexcept;


private:
    GeometryRegistry* pGeometryRegistry = nullptr;

	void processModelNodes(std::vector<Geometry>& geometries, const tinygltf::Model& model, const tinygltf::Node& node) noexcept;
	
    void processMesh(std::vector<Geometry>& geometries, const tinygltf::Model& model, const tinygltf::Mesh& mesh) noexcept;
};
}
