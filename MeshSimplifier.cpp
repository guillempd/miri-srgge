#include "Octree.h"
#include "PLYReader.h"
#include "PLYWriter.h"
#include "TriangleMesh.h"

#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

void computeRepresentatives(const TriangleMesh &originalMesh, Octree &octree, std::vector<OctreeNode*> &representative)
{
    for (int i = 0; i < originalMesh.triangles.size(); i += 3)
    {
        int i0 = originalMesh.triangles[i];
        int i1 = originalMesh.triangles[i + 1];
        int i2 = originalMesh.triangles[i + 2];

        glm::vec3 v0 = originalMesh.vertices[i0];
        glm::vec3 v1 = originalMesh.vertices[i1];
        glm::vec3 v2 = originalMesh.vertices[i2];

        representative[i0] = octree.insert(v0); // Add plane
        representative[i1] = octree.insert(v1); // Add plane
        representative[i2] = octree.insert(v2); // Add plane
    }
}

void addVertices(const TriangleMesh &originalMesh, TriangleMesh &simplifiedMesh, std::unordered_map<int, int> &originalToSimplifiedIndex, const std::vector<OctreeNode*> &representative, bool QEM) 
{
    std::unordered_map<OctreeNode*, int> simplifiedMeshVertices;
    int j = 0;
    for (int i = 0; i < originalMesh.vertices.size(); ++i)
    {
        bool vertex_found = (simplifiedMeshVertices.find(representative[i]) != simplifiedMeshVertices.end());
        if (!vertex_found)
        {
            if (QEM) simplifiedMesh.addVertex(Octree::QEM(representative[i]));
            else simplifiedMesh.addVertex(Octree::average(representative[i]));
            simplifiedMeshVertices[representative[i]] = j;
            originalToSimplifiedIndex[i] = j++;
        }
        else
        {
            originalToSimplifiedIndex[i] = simplifiedMeshVertices[representative[i]];
        }
    }
}

// TODO: Add const to originalToSimplifiedMesh
void addFaces(const TriangleMesh &originalMesh, TriangleMesh &simplifiedMesh, std::unordered_map<int, int> &originalToSimplifiedIndex)
{
    for (int i = 0; i < originalMesh.triangles.size(); i += 3)
    {
        int i0 = originalMesh.triangles[i];
        int i1 = originalMesh.triangles[i + 1];
        int i2 = originalMesh.triangles[i + 2];

        int j0 = originalToSimplifiedIndex[i0];
        int j1 = originalToSimplifiedIndex[i1];
        int j2 = originalToSimplifiedIndex[i2];

        // Skip face if it collapsed to an edge or point
        if (j0 == j1) continue;
        if (j1 == j2) continue;
        if (j2 == j0) continue;

        // Reorder indices so that they are in "cannonical" form to be hashed
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

        simplifiedMesh.addTriangle(j0, j1, j2);
    }
}

TriangleMesh ObtainQuadricErrorMethodLOD(const TriangleMesh &mesh, const std::vector<OctreeNode*> &representative)
{
    TriangleMesh simplifiedMesh;
    std::unordered_map<int, int> originalToSimplifiedIndex;
    addVertices(mesh, simplifiedMesh, originalToSimplifiedIndex, representative, true);
    addFaces(mesh, simplifiedMesh, originalToSimplifiedIndex);
    return simplifiedMesh;
}

TriangleMesh ObtainAverageLOD(const TriangleMesh &mesh, const std::vector<OctreeNode*> &representative)
{
    TriangleMesh simplifiedMesh;
    std::unordered_map<int, int> originalToSimplifiedIndex;
    addVertices(mesh, simplifiedMesh, originalToSimplifiedIndex, representative, false);
    addFaces(mesh, simplifiedMesh, originalToSimplifiedIndex);
    return simplifiedMesh;
}

// TODO: Specify amount of levels of detail
std::vector<TriangleMesh> SimplifyMesh(const TriangleMesh &mesh)
{
    Octree octree(mesh.aabb);
    std::vector<OctreeNode*> representative(mesh.vertices.size(), nullptr);
    computeRepresentatives(mesh, octree, representative);

    std::vector<TriangleMesh> LOD;
    for (int l = 0; l < 4; ++l)
    {
        LOD.push_back(ObtainAverageLOD(mesh, representative));
        for (int i = 0; i < mesh.vertices.size(); ++i)
        {
            representative[i] = representative[i]->parent;
        }
    }
    return LOD;
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
        std::vector<TriangleMesh> LOD = SimplifyMesh(mesh);
        for (int i = 0; i < LOD.size(); ++i)
            PLYWriter::writeMesh("test" + std::to_string(i) + ".ply", LOD[i]);
    }
    else
    {
        std::cerr << "Failed to load " + mesh_filename << std::endl;
        return -1;
    }
}