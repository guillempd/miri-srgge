#include "aabb.cpp"

#include <array>

using Octree = OctreeNode *;

struct OctreeChilds
{
    std::array<OctreeNode*, 8> childs; 
};

struct OctreeNode
{
    // Information at each node (?)
    OctreeChilds *childs;
};