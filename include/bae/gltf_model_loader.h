#pragma once
#include <string>

namespace bae {
	struct Model;

	Model loadGltfModel(const std::string& assetPath, const std::string& fileName);
}