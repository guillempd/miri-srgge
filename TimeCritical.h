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
