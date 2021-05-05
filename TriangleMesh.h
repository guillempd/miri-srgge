#ifndef _TRIANGLE_MESH_INCLUDE
#define _TRIANGLE_MESH_INCLUDE

#include "AABB.h"
#include "ShaderProgram.h"

#include <glm/glm.hpp>

#include <vector>

class TriangleMesh
{

public:
    TriangleMesh();

    void addVertex(const glm::vec3 &position);
    void addTriangle(int v0, int v1, int v2);

    void buildCube();

    void sendToOpenGL(ShaderProgram &program);
    void render() const;
    void free();

    std::vector<glm::vec3> vertices;
    std::vector<int> triangles;

    AABB aabb;

private:

    GLuint vao;
    GLuint vbo;
    GLint posLocation, normalLocation;
};

#endif // _TRIANGLE_MESH_INCLUDE
