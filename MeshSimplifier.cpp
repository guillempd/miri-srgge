#include "PLYReader.h"
#include "PLYWriter.h"
#include "TriangleMesh.h"

#include <iostream>
#include <string>
using namespace std;

const string model_filename = "../../models/torus.ply";

int main(int argc, char **argv)
{
    TriangleMesh mesh;
    if (PLYReader::readMesh(model_filename, mesh))
        PLYWriter::writeMesh("../test.ply", mesh);
    else cerr << "Failed to load " + model_filename << endl;
}