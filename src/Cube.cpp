#include <vector>

#include "utils/Vertex.h"

namespace bae
{
const std::vector<bae::PosColorVertex> cubeVertices{
    {glm::vec3{-1.0f, 1.0f, 1.0f}, 0xff000000},
    {glm::vec3{1.0f, 1.0f, 1.0f}, 0xff0000ff},
    {glm::vec3{-1.0f, -1.0f, 1.0f}, 0xff00ff00},
    {glm::vec3{1.0f, -1.0f, 1.0f}, 0xffff0000},
    {glm::vec3{-1.0f, 1.0f, -1.0f}, 0xff00ffff},
    {glm::vec3{1.0f, 1.0f, -1.0f}, 0xffff00ff},
    {glm::vec3{-1.0f, -1.0f, -1.0f}, 0xffffff00},
    {glm::vec3{1.0f, -1.0f, -1.0f}, 0xffffffff},
};

const std::vector<uint16_t> cubeIndices{
    0, 1,
    2,  // 0
    1, 3,
    2,  // 1
    4, 6,
    5,  // 2
    5, 6,
    7,  // 3
    0, 2,
    4,  // 4
    4, 2,
    6,  // 5
    1, 5,
    3,  // 6
    5, 7,
    3,  // 7
    0, 4,
    1,  // 8
    4, 5,
    1,  // 9
    2, 3,
    6,  // 10
    6, 3,
    7,  // 11
};

}  // namespace bae
