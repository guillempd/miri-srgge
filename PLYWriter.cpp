#include "PLYWriter.h"

bool PLYWriter::writeMesh(const std::string &filename, const TriangleMesh &mesh)
{
    std::ofstream fout(filename);
    writeHeader(fout, mesh);
    writeVertices(fout, mesh);
    writeFaces(fout, mesh);
    return true;
}

void PLYWriter::writeHeader(std::ofstream &fout, const TriangleMesh &mesh)
{
    fout << "ply\n";
    fout << "format binary_little_endian 1.0\n";
    fout << "element vertex " << mesh.vertices.size() << "\n";
    fout << "property float x\n";
    fout << "property float y\n";
    fout << "property float z\n";
    fout << "element face " << mesh.triangles.size() / 3 << "\n";
    fout << "property list uchar int vertex_index\n";
    fout << "end_header\n";
}

void PLYWriter::writeVertices(std::ofstream &fout, const TriangleMesh &mesh)
{
    for (int i = 0; i < mesh.vertices.size(); ++i) {
        glm::vec3 vertex = mesh.vertices[i];
        fout.write(reinterpret_cast<char*>(&vertex.x), sizeof(float));
        fout.write(reinterpret_cast<char*>(&vertex.y), sizeof(float));
        fout.write(reinterpret_cast<char*>(&vertex.z), sizeof(float));
    }
}

void PLYWriter::writeFaces(std::ofstream &fout, const TriangleMesh &mesh)
{
    for (int i = 0; i < mesh.triangles.size(); i += 3) {
        unsigned char n = 3;
        int i0 = mesh.triangles[i];
        int i1 = mesh.triangles[i + 1];
        int i2 = mesh.triangles[i + 2];
        fout.write(reinterpret_cast<char*>(&n), sizeof(unsigned char));
        fout.write(reinterpret_cast<char*>(&i0), sizeof(int));
        fout.write(reinterpret_cast<char*>(&i1), sizeof(int));
        fout.write(reinterpret_cast<char*>(&i2), sizeof(int));
    }
}