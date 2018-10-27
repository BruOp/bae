#pragma once

#include <glm/glm.hpp>

namespace bae
{

typedef glm::vec3 Position;
typedef glm::vec3 Direction;

glm::vec3 crossAndNormalize(const glm::vec3 &u, const glm::vec3 &v);
} // namespace bae
