#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>

#include <limits>

struct AABB
{
    glm::vec3 min;
    glm::vec3 max;

    AABB(glm::vec3 vertex) : min(vertex), max(vertex) {}
    AABB(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}
    AABB() : AABB(glm::vec3(std::numeric_limits<float>::max()), -glm::vec3(std::numeric_limits<float>::max())) {}
};

#endif // AABB_H