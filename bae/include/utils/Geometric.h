#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace bae
{

typedef glm::vec3 Position;
typedef glm::vec3 Direction;
typedef glm::quat Rotation;
typedef glm::mat4 Transform;

struct LinkedTransform {
    Transform local;
    uint32_t parent;
};

glm::vec3 crossAndNormalize(const glm::vec3 &u, const glm::vec3 &v);
} // namespace bae
