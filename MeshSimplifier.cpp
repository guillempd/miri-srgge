#include "Octree.h"
#include "PLYReader.h"
#include "PLYWriter.h"
#include "TriangleMesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

enum SimplificationMethod
{
    MEAN,
    QEM,
    THIN_FEATURE
};

Plane face(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
{
    glm::vec3 u = v1 - v0;
    glm::vec3 v = v2 - v0;
    glm::vec3 n = glm::normalize(glm::cross(u, v));
    float d = -glm::dot(v0, n);
    Plane result;
    result << n.x, n.y, n.z, d;
    return result;
}

void computeRepresentativesByVertices(const TriangleMesh &originalMesh, Octree &octree, std::vector<OctreeNode*> &representative)
{
    for (int i = 0; i < originalMesh.vertices.size(); ++i)
    {
        glm::vec3 v = originalMesh.vertices[i];
        representative[i] = octree.insert(v, Plane(0, 0, 0, 0)); // FIXME: Inserting dummy face
    }
}

void computeRepresentativesByCorners(const TriangleMesh &originalMesh, Octree &octree, std::vector<OctreeNode*> &representative)
{
    for (int i = 0; i < originalMesh.triangles.size(); i += 3)
    {
        int i0 = originalMesh.triangles[i];
        int i1 = originalMesh.triangles[i + 1];
        int i2 = originalMesh.triangles[i + 2];

        glm::vec3 v0 = originalMesh.vertices[i0];
        glm::vec3 v1 = originalMesh.vertices[i1];
        glm::vec3 v2 = originalMesh.vertices[i2];

        Plane triangle = face(v0, v1, v2);

        representative[i0] = octree.insert(v0, triangle);
        representative[i1] = octree.insert(v1, triangle);
        representative[i2] = octree.insert(v2, triangle);
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
    std::unordered_set<glm::ivec3> simplifiedMeshTriangles;
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
        
        glm::ivec3 triangle(j0, j1, j2);
        bool triangle_found = (simplifiedMeshTriangles.find(triangle) != simplifiedMeshTriangles.end());
        if (!triangle_found)
        {
            simplifiedMesh.addTriangle(j0, j1, j2);
            simplifiedMeshTriangles.insert(triangle);
        }
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

std::vector<TriangleMesh> SimplifyMesh(const TriangleMesh &mesh, SimplificationMethod method, int max_depth, int lods)
{
    Octree octree(mesh.aabb, max_depth); // TODO: Pass in max_depth
    std::vector<OctreeNode*> representative(mesh.vertices.size(), nullptr);

    if (method == QEM) computeRepresentativesByCorners(mesh, octree, representative);
    else computeRepresentativesByVertices(mesh, octree, representative);

    std::vector<TriangleMesh> LOD;
    for (int l = 0; l < lods; ++l)
    {
        switch (method)
        {
            case QEM:
                LOD.push_back(ObtainQuadricErrorMethodLOD(mesh, representative));
                break;

            default:
                std::cerr << "E: Unknown simplification method, 'mean' method selected" << std::endl;
                // Intentional fallthrough
            case MEAN:
                LOD.push_back(ObtainAverageLOD(mesh, representative));
                break;
        }
        for (int i = 0; i < mesh.vertices.size(); ++i)
        {
            representative[i] = representative[i]->parent;
        }
    }
    return LOD;
}

const std::string DEFAULT_MESH = "models/bunny.ply";

const SimplificationMethod DEFAULT_METHOD = MEAN;

const int MIN_MAX_DEPTH = 6;
const int MAX_MAX_DEPTH = 12;
const int DEFAULT_MAX_DEPTH = 8;

const int MIN_LODS = 1;
const int DEFAULT_LODS = 4;

int main(int argc, char **argv)
{
    std::string mesh_filename = DEFAULT_MESH;
    if (argc > 1)
    {
        mesh_filename = std::string(argv[1]);
    }

    SimplificationMethod method = DEFAULT_METHOD;
    if (argc > 2)
    {
        std::string input_method = std::string(argv[2]);
        if (input_method == "mean") method = MEAN;
        else if (input_method == "qem") method = QEM;
        else
        {
            std::cerr << "W: Unknown simplification method." << std::endl;
            std::cerr << "W: Available ones are: 'mean' and 'qem'" << std::endl;
            std::cerr << "W: Defaulted to 'mean'" << std::endl;
        }
    }

    int max_depth = DEFAULT_MAX_DEPTH;
    if (argc > 3)
    {
        int input_max_depth = std::atoi(argv[3]);
        if (MIN_MAX_DEPTH <= input_max_depth && input_max_depth <= MAX_MAX_DEPTH)
            max_depth = input_max_depth;
        else
        {
            std::cerr << "W: max_depth has to be between " << MIN_MAX_DEPTH << " and " << MAX_MAX_DEPTH << std::endl;
            std::cerr << "W: Defaulted to " << DEFAULT_MAX_DEPTH << std::endl;
        }
    }

    int lods = DEFAULT_LODS;
    if (argc > 4)
    {
        int input_lods = std::atoi(argv[4]);
        if (MIN_LODS <= input_lods && input_lods <= max_depth)
            lods = input_lods;
        else
        {
            std::cerr << "W: lods has to be between " << MIN_LODS << " and max_depth (" << max_depth << ")" <<  std::endl;
            std::cerr << "W: Defaulted to " << MIN_LODS << std::endl;
        }
    }

    TriangleMesh mesh;
    if (PLYReader::readMesh(mesh_filename, mesh))
    {
        std::vector<TriangleMesh> LOD = SimplifyMesh(mesh, method, max_depth, lods);
        for (int i = 0; i < LOD.size(); ++i)
            PLYWriter::writeMesh("test" + std::to_string(i) + ".ply", LOD[i]); // TODO: Change generated names
    }
    else
    {
        std::cerr << "Failed to load " + mesh_filename << std::endl;
        return -1;
    }
}