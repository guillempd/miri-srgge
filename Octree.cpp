#include "Octree.h"

#include <bitset>

Octree::Octree() : max_depth (6)
{
    universe.center = glm::vec3(0.0f);
    universe.half_length = glm::vec3(0.5f);
    root.average = glm::vec3(0.0f);
    root.n = 0;
    root.children = nullptr;
}

Octree::~Octree()
{
    free(root);
}

OctreeNode* Octree::insert(const glm::vec3 &vertex)
{
    AABB current_universe = universe;
    OctreeNode *current_node = &root;
    OctreeNode *representative = current_node;
    update_node(current_node, vertex);
    for (int depth = 0; depth < max_depth; ++depth)
    {
        AABB next_universe;
        next_universe.half_length = current_universe.half_length / 2.0f;
        next_universe.center = current_universe.center - next_universe.half_length;
        std::bitset<3> child_index;
        if (vertex.x > current_universe.center.x) 
        {
            child_index.set(0);
            next_universe.center.x += 2.0f * next_universe.half_length.x; 
        }
        if (vertex.y > current_universe.center.y) 
        {
            child_index.set(1);
            next_universe.center.y += 2.0f * next_universe.half_length.y;
        }
        if (vertex.z > current_universe.center.z) 
        {
            child_index.set(2);
            next_universe.center.z  += 2.0f * next_universe.half_length.z;
        }
        current_universe.center = next_universe.center;
        current_universe.half_length = next_universe.half_length;
        if (!current_node->children) subdivide(current_node);
        current_node = &current_node->children->node[child_index.to_ulong()];
        representative = current_node; // TODO: Obtain multiple representatives
        update_node(current_node, vertex);
    }
    return representative;
}

void Octree::update_node(OctreeNode *node, const glm::vec3 &vertex)
{
    node->average = node->average * (float(node->n) / float(node->n + 1)) + vertex / float(node->n + 1);
    node->n += 1;
}

void Octree::subdivide(OctreeNode *node)
{
    node->children = new OctreeChildren { }; // TODO: Correctly initialize this
    for (int i = 0; i < 8; ++i) {
        OctreeNode &child_node = node->children->node[i];
        child_node.average = glm::vec3(0.0f);
        child_node.n = 0;
        child_node.children = nullptr;
    }
}

void Octree::free(OctreeNode &node)
{
    OctreeChildren *children = node.children;
    if (children)
    {
        for (int i = 0; i < children->node.size(); ++i) free(children->node[i]);
        delete children;
    }
}