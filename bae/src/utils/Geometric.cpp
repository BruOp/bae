#include "utils/Geometric.h"

namespace bae
{
glm::vec3 crossAndNormalize(const glm::vec3 &u, const glm::vec3 &v)
{
    return glm::normalize(glm::cross(u, v));
}
} // namespace bae
