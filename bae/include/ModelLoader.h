#pragma once
#include <entt/entt.hpp>
#include <exception>
#include <glm/gtc/quaternion.hpp>

#include "Geometry.h"
#include "utils/Common.h"
#include "utils/Vertex.h"

namespace bae
{
class ModelLoader
{
   public:
    ModelLoader() = default;
    ModelLoader(GeometryRegistry* geoRegistry) : pGeometryRegistry{geoRegistry} {};

    Geometry loadObjModel(const std::string& name, const std::string& file);

   private:
    GeometryRegistry* pGeometryRegistry = nullptr;
};
}  // namespace bae
