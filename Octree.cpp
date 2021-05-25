#include "Octree.h"

#include <bitset>

Octree::Octree()
    : root()
    , center(0.0f)
    , half_length(0.5f)
    , max_depth(4)
    {
        root.is_leaf = true;
        root.pointer.data = nullptr;
        root.parent = nullptr;
    }

Octree::Octree(AABB aabb, int max_depth_)
    : root()
    , center((aabb.min + aabb.max)/ 2.0f)
    , half_length(compute_half_length(aabb))
    , max_depth(max_depth_)
    {
        root.is_leaf = true;
        root.pointer.data = nullptr;
        root.parent = nullptr;
    }

Octree::~Octree()
{
    if (!root.is_leaf) free(root.pointer.children);
}

OctreeNode* Octree::insert(const glm::vec3 &vertex, const Plane &face)
{
    glm::vec3 current_center = center;
    float current_half_length = half_length;
    OctreeNode *current_node = &root;
    for (int depth = 0; depth < max_depth; ++depth)
    {
        float next_half_length = current_half_length / 2.0f;
        glm::vec3 next_center = current_center - next_half_length;
        std::bitset<3> child_index;
        if (vertex.x > current_center.x) 
        {
            child_index.set(0);
            next_center.x += 2.0f * next_half_length; 
        }
        if (vertex.y > current_center.y) 
        {
            child_index.set(1);
            next_center.y += 2.0f * next_half_length;
        }
        if (vertex.z > current_center.z) 
        {
            child_index.set(2);
            next_center.z  += 2.0f * next_half_length;
        }
        current_center = next_center;
        current_half_length = next_half_length;
        if (current_node->is_leaf) subdivide(current_node);
        current_node = &current_node->pointer.children->node[child_index.to_ulong()];
    }
    insert(current_node->pointer.data, vertex, face);
    return current_node;
}

void Octree::insert(OctreeData *&data, const glm::vec3 &vertex, const Plane &face)
{
    if (!data) data = new OctreeData {vertex, 1};
    else 
    {
        float weight = float(data->vertices) / float(data->vertices + 1);
        data->average = data->average * weight + vertex * (1.0f - weight);
        data->vertices += 1;
        data->faces.push_back(face);
    }   
}

glm::vec3 Octree::average(OctreeNode *node)
{
    if (!node->is_leaf) aggregate(node);
    return node->pointer.data->average;
}

glm::vec3 Octree::QEM(OctreeNode *node)
{
    if (!node->is_leaf) aggregate(node);
    // return node->pointer.data->average;

    using Matrix4 = Eigen::Matrix4d;
    using Vector4 = Eigen::Vector4d;
    using JacobiSVD = Eigen::JacobiSVD<Matrix4>;

    Matrix4 Q = Matrix4::Zero();
    const std::list<Plane> &faces = node->pointer.data->faces;
    for (const auto &face : faces)
    {
        Q += (face * face.transpose()).cast<double>();
    }
    Q(3,0) = 0;
    Q(3,1) = 0;
    Q(3,2) = 0;
    Q(3,3) = 1;

    Vector4 b(0, 0, 0, 1);

    JacobiSVD svd(Q, Eigen::ComputeFullU | Eigen::ComputeFullV);
    if (svd.rank() < 4) return average(node); // TODO: Improve this checking for rank 3 (case of edges)
    else
    {
        Vector4 result = svd.solve(b);
        return glm::vec3(result(0), result(1), result(2));
    }
}

// !node->is_leaf
void Octree::aggregate(OctreeNode *node)
{
    OctreeChildren *children = node->pointer.children;
    OctreeData *data = new OctreeData {glm::vec3(0.0f), 0}; // TODO: Correctly initialize this
    node->pointer.data = data;
    node->is_leaf = true;
    for (int i = 0; i < children->node.size(); ++i)
    {
        OctreeNode &child = children->node[i];
        aggregate(data, child.pointer.data);
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
        parent->faces.splice(parent->faces.end(), child->faces);
        delete child;
    }
}

// node is not null
void Octree::subdivide(OctreeNode *node)
{
    OctreeChildren *children = new OctreeChildren;
    node->pointer.children = children;
    for (int i = 0; i < children->node.size(); ++i)
    {
        OctreeNode &child = children->node[i];
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

float Octree::compute_half_length(const AABB &aabb)
{
    glm::vec3 length = aabb.max - aabb.min;
    float half_length = std::max(length.x, std::max(length.y, length.z)) / 2.0f;
    return 1.1f * half_length;
}
