#include "TriangleMesh.h"

#include "glm/glm.hpp"

#include <array>

// lod -> subdivision level
// 0 -> 8
// 1 -> 7
// 2 -> 6
// 3 -> 5
struct MeshLods
{
    std::array<TriangleMesh, 4> lods;
};


struct Statue
{
    const MeshLods &meshLods;
    glm::ivec2 position;
};