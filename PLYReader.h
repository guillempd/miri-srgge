#ifndef PLYREADER_H
#define PLYREADER_H

#include <fstream>
#include <string>
#include <vector>

#include "TriangleMesh.h"

class PLYReader
{

public:
    static bool readMesh(const std::string &filename, TriangleMesh &mesh);

private:
    static bool loadHeader(std::ifstream &fin, int &nVertices, int &nFaces);
    static void loadVertices(std::ifstream &fin, int nVertices, std::vector<float> &plyVertices);
    static void loadFaces(std::ifstream &fin, int nFaces, std::vector<int> &plyTriangles);
    static void rescaleModel(std::vector<float> &plyVertices);
    static void addModelToMesh(const std::vector<float> &plyVertices, const std::vector<int> &plyTriangles, TriangleMesh &mesh);
};

#endif // PLYREADER_H
