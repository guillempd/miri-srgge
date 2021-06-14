#include "TriangleMesh.h"

#include "glm/glm.hpp"

#include <array>

// lod -> subdivision level
// 0 -> 5
// 1 -> 6
// 2 -> 7
// 3 -> 8
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

static float value(const Assignment &assignment)
{
    return assignment.benefit / assignment.cost;    
}

struct AssignmentPriority
{
    bool operator()(const Assignment &lhs, const Assignment &rhs) {
        return value(lhs) < value(rhs);
    }
};
