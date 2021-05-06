#ifndef OCTREE_H
#define OCTREE_H

#include "AABB.h"

#include <Eigen/Eigen> // TODO: Improve this by including minimal headers

#include <glm/glm.hpp>

#include <array>
#include <list>

using Plane = Eigen::Vector4f;

struct OctreeNode;
struct OctreeChildren;

struct OctreeData
{
    glm::vec3 average;
    int vertices;
    std::list<Plane> faces; // TODO: Be careful with Eigen <-> STL interaction
};

union OctreePointer
{
    OctreeData *data;
    OctreeChildren *children;
};

struct OctreeNode
{
    OctreePointer pointer;
    OctreeNode *parent;
    bool is_leaf;
};

struct OctreeChildren
{
    std::array<OctreeNode, 8> node;
};

class Octree 
{
public:
    Octree();
    Octree(AABB aabb);
    ~Octree();
    OctreeNode* insert(const glm::vec3 &vertex, const Plane &face);
    static glm::vec3 average(OctreeNode *node); // TODO: Make non-static (?)
    static glm::vec3 QEM(OctreeNode *node);

private:
    OctreeNode root;
    glm::vec3 center;
    glm::vec3 half_length;
    const int max_depth;

private:
    static void free(OctreeNode *root);
    static void free(OctreeChildren *children);
    static void insert(OctreeData *&data, const glm::vec3 &vertex, const Plane &face);
    static void subdivide(OctreeNode *node);
    static void aggregate(OctreeNode *parent, OctreeChildren *children);
    static void aggregate(OctreeData *parent, OctreeData *child);
};

#endif // OCTREE_H