#include "Octree.h"
#include "PLYReader.h"
#include "PLYWriter.h"
#include "TriangleMesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp> // Experimental feature to provide hashing capabilities

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

// TODO: Take into account the multiple LODS
TriangleMesh SimplifyMesh(const TriangleMesh &mesh)
{
    TriangleMesh simplifiedMesh;
    Octree octree(mesh.aabb);
    std::vector<OctreeNode*> representative(mesh.vertices.size(), nullptr);
    // Compute representatives
    for (int i = 0; i < mesh.vertices.size(); ++i)
    {
        glm::vec3 vertex = mesh.vertices[i];
        representative[i] = octree.insert(vertex);
    }

    // Add representatives to simplifiedMesh
    // If needed (QEM) compute them
    std::unordered_map<OctreeNode*, int> simplifiedMeshVertices;
    std::unordered_map<int, int> originalToSimplifiedIndex;
    int j = 0;
    for (int i = 0; i < mesh.vertices.size(); ++i)
    {
        bool vertex_found = (simplifiedMeshVertices.find(representative[i]) != simplifiedMeshVertices.end());
        if (!vertex_found)
        {
            simplifiedMesh.addVertex(representative[i]->pointer.data->average);
            simplifiedMeshVertices[representative[i]] = j;
            originalToSimplifiedIndex[i] = j++;
        }
        else
        {
            originalToSimplifiedIndex[i] = simplifiedMeshVertices[representative[i]];
        }
    }

    // Add faces to simplifiedMesh
    std::unordered_set<glm::ivec3> simplifiedMeshTriangles;
    for (int i = 0; i < mesh.triangles.size(); i += 3)
    {
        int i0 = mesh.triangles[i];
        int i1 = mesh.triangles[i + 1];
        int i2 = mesh.triangles[i + 2];

        int j0 = originalToSimplifiedIndex[i0];
        int j1 = originalToSimplifiedIndex[i1];
        int j2 = originalToSimplifiedIndex[i2];

        // If face collapsed to edge or point skip
        if (j0 == j1) continue;
        if (j1 == j2) continue;
        if (j2 == j0) continue;

        // Reorder indices so the first one is the smallest
        if (j1 < j0 && j1 < j2)
        {
            int aux = j0;
            j0 = j1;
            j1 = j2;
            j2 = aux;
        }
        else if (j2 < j0 && j2 < j1)
        {
            int aux = j0;
            j0 = j2;
            j2 = j1;
            j1 = aux;
        }

        glm::ivec3 triangle(j0, j1, j2);
        bool triangle_found = (simplifiedMeshTriangles.find(triangle) != simplifiedMeshTriangles.end());
        if (!triangle_found)
        {
            simplifiedMesh.addTriangle(j0, j1, j2);
            simplifiedMeshTriangles.insert(triangle);
        }
    }
    return simplifiedMesh;
}

std::string fallback_mesh_filename = "models/torus.ply";

int main(int argc, char **argv)
{
    std::string mesh_filename = fallback_mesh_filename;
    if (argc > 1)
    {
        mesh_filename = std::string(argv[1]);
    }

    TriangleMesh mesh;
    if (PLYReader::readMesh(mesh_filename, mesh))
    {
        TriangleMesh simplifiedMesh = SimplifyMesh(mesh);
        PLYWriter::writeMesh("test.ply", simplifiedMesh);
    }
    else
    {
        std::cerr << "Failed to load " + mesh_filename << std::endl;
        return -1;
    }
}