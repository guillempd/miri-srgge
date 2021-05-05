#ifndef OCTREE_H
#define OCTREE_H

#include <glm/glm.hpp>

#include <array>

using Data = int;

struct AABB
{
    glm::vec3 center;
    glm::vec3 half_length;
};

struct OctreeNode;
struct OctreeChildren;

struct OctreeNode
{
    glm::vec3 average;
    int n;
    OctreeChildren *children;
};

struct OctreeChildren
{
    std::array<OctreeNode, 8> node;
};

class Octree 
{

public:

    Octree();
    ~Octree();
    OctreeNode* insert(const glm::vec3 &vertex);

private:
    OctreeNode root;
    AABB universe;
    const int max_depth;

private:
    static void update_node(OctreeNode *node, const glm::vec3 &vertex);
    static void subdivide(OctreeNode *node);
    static void free(OctreeNode &node);

};

#endif // OCTREE_H