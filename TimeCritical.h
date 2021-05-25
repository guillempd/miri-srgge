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

struct Assignment
{
    int index;
    int lod;
    float benefit;
    float cost;
};

struct AssignmentPriority
{
    bool operator()(const Assignment &lhs, const Assignment &rhs) {
        float lhs_value = lhs.benefit / lhs.cost;
        float rhs_value = rhs.benefit / rhs.cost;
        return lhs_value < rhs_value;
    }
};