#include "Octree.h"
#include "PLYReader.h"
#include "PLYWriter.h"
#include "TriangleMesh.h"

#include <iostream>
#include <string>
#include <unordered_map>

// TODO: Take into account the multiple LODS
TriangleMesh SimplifyMesh(const TriangleMesh &mesh)
{
    TriangleMesh simplifiedMesh;
    Octree octree;
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
    int j = 0;
    for (int i = 0; i < mesh.vertices.size(); ++i)
    {
        bool vertex_found = (simplifiedMeshVertices.find(representative[i]) != simplifiedMeshVertices.end());
        if (!vertex_found)
        {
            simplifiedMesh.addVertex(representative[i]->average);
            simplifiedMeshVertices[representative[i]] = j++;
        }
    }

    // Add faces to simplifiedMesh
    // TODO: Removing duplicates
    for (int i = 0; i < mesh.triangles.size(); i += 3)
    {
        int i0 = mesh.triangles[i];
        int i1 = mesh.triangles[i + 1];
        int i2 = mesh.triangles[i + 2];
        simplifiedMesh.addTriangle(
            simplifiedMeshVertices[representative[i0]],
            simplifiedMeshVertices[representative[i1]],
            simplifiedMeshVertices[representative[i2]]
            );
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