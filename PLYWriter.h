#ifndef PLYWRITER_H
#define PLYWRITER_H

#include "TriangleMesh.h"

#include <fstream>
#include <string>

class PLYWriter
{
public:
    static bool writeMesh(const std::string &filename, const TriangleMesh &mesh);

private:
    static void writeHeader(std::ofstream &fout, const TriangleMesh &mesh);
    static void writeVertices(std::ofstream &fout, const TriangleMesh &mesh);
    static void writeFaces(std::ofstream &fout, const TriangleMesh &mesh);
};

#endif // PLYWRITER_H