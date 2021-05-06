#include "Octree.h"

#include <bitset>

Octree::Octree()
    : center(0.0f)
    , half_length(0.5f)
    , max_depth(4)
    {
        root.is_leaf = true;
        root.pointer.data = nullptr;
    }

Octree::Octree(AABB aabb)
    : max_depth(7)
    , center((aabb.min + aabb.max)/ 2.0f)
    , half_length((aabb.max - aabb.min) / 2.0f)
    {
        root.is_leaf = true;
        root.pointer.data = nullptr;
    }

Octree::~Octree()
{
    // if (!root.is_leaf) free(root.pointer.children);
}

OctreeNode* Octree::insert(const glm::vec3 &vertex) // TODO: Change representative type
{
    glm::vec3 current_center = center;
    glm::vec3 current_half_length = half_length;
    OctreeNode *current_node = &root;
    for (int depth = 0; depth < max_depth; ++depth)
    {
        glm::vec3 next_half_length = current_half_length / 2.0f;
        glm::vec3 next_center = current_center - next_half_length;
        std::bitset<3> child_index;
        if (vertex.x > current_center.x) 
        {
            child_index.set(0);
            next_center.x += 2.0f * next_half_length.x; 
        }
        if (vertex.y > current_center.y) 
        {
            child_index.set(1);
            next_center.y += 2.0f * next_half_length.y;
        }
        if (vertex.z > current_center.z) 
        {
            child_index.set(2);
            next_center.z  += 2.0f * next_half_length.z;
        }
        current_center = next_center;
        current_half_length = next_half_length;
        if (current_node->is_leaf) subdivide(current_node);
        current_node = &current_node->pointer.children->node[child_index.to_ulong()];
    }
    insert(current_node->pointer.data, vertex);
    return current_node;
}

void Octree::insert(OctreeData *&data, const glm::vec3 &vertex)
{
    if (!data) data = new OctreeData {vertex, 1};
    else 
    {
        float weight = float(data->vertices) / float(data->vertices + 1);
        data->average = data->average * weight + vertex * (1.0f - weight);
        data->vertices += 1;
    }   
}

glm::vec3 Octree::average(OctreeNode *node)
{
    if (!node->is_leaf) aggregate(node, node->pointer.children);
    return node->pointer.data->average;
}

// children->parent is not null
void Octree::aggregate(OctreeNode *parent, OctreeChildren *children)
{
    OctreeData *parent_data = new OctreeData {glm::vec3(0.0f), 0}; // TODO: Correctly initialize this
    parent->pointer.data = parent_data;
    for (int i = 0; i < children->node.size(); ++i)
    {
        OctreeNode *child = &children->node[i];
        aggregate(parent_data, child->pointer.data);
    }
    delete children;
}

// parent is not null
void Octree::aggregate(OctreeData *parent, OctreeData *child)
{
    if (child)
    {
        float parent_weight = float(parent->vertices) / float(parent->vertices + child->vertices);
        parent->average = parent->average * parent_weight + child->average * (1.0f - parent_weight);
        parent->vertices += child->vertices;
        delete child;
    }
}

// node is not null
void Octree::subdivide(OctreeNode *node)
{
    node->pointer.children = new OctreeChildren; // TODO: Correctly initialize this
    for (int i = 0; i < node->pointer.children->node.size(); ++i)
    {
        OctreeNode &child = node->pointer.children->node[i];
        child.is_leaf = true;
        child.pointer.data = nullptr;
        child.parent = node;
    }
    node->is_leaf = false;
}

void Octree::free(OctreeNode *node)
{
    if (node->is_leaf)
    {
        if (node->pointer.data) delete node->pointer.data;
    }
    else free(node->pointer.children);
}

void Octree::free(OctreeChildren *children)
{
    for (int i = 0; i < children->node.size(); ++i)
    {
        free(&children->node[i]);
    }
    delete children;
}