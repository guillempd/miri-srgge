#include "Scene.h"
#include "PLYReader.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <iostream>
#include <fstream>
#include <string>

Scene::Scene()
{
    meshes = {};
}

// TODO: Cleanup all the meshes
Scene::~Scene()
{
    for (auto mesh : meshes)
        if (mesh) delete mesh;
}

void Scene::init()
{
    initShaders();
    meshes.push_back(new TriangleMesh());
    meshes[0]->buildCube();
    meshes[0]->sendToOpenGL(basicProgram);

    currentTime = 0.0f;

    camera.init();

    bPolygonFill = true;
    
    tilemap = {{}};
    tile = std::vector<unsigned char>(256, 0);
}

// bool Scene::loadMesh(const char *filename)
// {
//     PLYReader reader;

//     mesh->free();
//     bool bSuccess = reader.readMesh(filename, *mesh);
//     if (bSuccess)
//         mesh->sendToOpenGL(basicProgram);

//     return bSuccess;
// }

bool Scene::loadScene(const char *filename)
{
    std::ifstream fin(filename, std::ios::in);
    if (!fin.is_open()) return false;

    int n;
    fin >> n;
    int j = 0;
    for (int i = 0; i < n; ++i)
    {
        char c;
        std::string meshFilename;
        fin >> c >> meshFilename;

        TriangleMesh *mesh = new TriangleMesh();
        PLYReader reader;
        bool success = reader.readMesh(meshFilename.c_str(), *mesh);
        if (success) 
        {
            mesh->sendToOpenGL(basicProgram);
            meshes.push_back(mesh);
            tile[c] = ++j;
        }
        else delete mesh;
    }
    int w, h;
    fin >> w >> h;
    tilemap = std::vector<std::vector<unsigned char>> (w, std::vector<unsigned char>(h));
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            fin >> tilemap[x][y];
        }
    }
    return true;
}

void Scene::update(int deltaTime)
{
    currentTime += deltaTime;
    camera.update(deltaTime);
}

void Scene::render(int n)
{
    int w = tilemap.size();
    int h = tilemap[0].size();
    const glm::mat4 &viewMatrix = camera.getViewMatrix();
    const glm::mat4 &projectionMatrix = camera.getProjectionMatrix();
    basicProgram.use();
    basicProgram.setUniformMatrix4f("view", viewMatrix);
    basicProgram.setUniformMatrix4f("projection", projectionMatrix);
    basicProgram.setUniform1i("bLighting", bPolygonFill ? 1 : 0);
    if (bPolygonFill)
    {
        basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else
    {
        basicProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.5f, 1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        for (int x = 0; x < w; ++x)
            for (int y = 0; y < h; ++y)
                render(x, y, viewMatrix);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_POLYGON_OFFSET_FILL);
        basicProgram.setUniform4f("color", 0.0f, 0.0f, 0.0f, 1.0f);
    }
    for (int x = 0; x < w; ++x)
        for (int y = 0; y < h; ++y)
            render(x, y, viewMatrix);            
}

void Scene::render(int x, int y, const glm::mat4 &viewMatrix)
{
    unsigned char b = tilemap[x][y];
    unsigned char i = tile[b];
    if (i >= 0 && b != '.')
    {
        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x, 0, y));
        basicProgram.setUniformMatrix4f("model", modelMatrix);

        glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(viewMatrix * modelMatrix));
        basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);

        meshes[i]->render();
    }
    // TODO: Render floor in any case
}

Camera &Scene::getCamera()
{
    return camera;
}

void Scene::switchPolygonMode()
{
    bPolygonFill = !bPolygonFill;
}

void Scene::initShaders()
{
    Shader vShader, fShader;

    vShader.initFromFile(VERTEX_SHADER, "shaders/basic.vs");
    if (!vShader.isCompiled())
    {
        std::cout << "Vertex Shader Error" << std::endl;
        std::cout << "" << vShader.log() << std::endl << std::endl;
    }
    fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.fs");
    if (!fShader.isCompiled())
    {
        std::cout << "Fragment Shader Error" << std::endl;
        std::cout << "" << fShader.log() << std::endl << std::endl;
    }
    basicProgram.init();
    basicProgram.addShader(vShader);
    basicProgram.addShader(fShader);
    basicProgram.link();
    if (!basicProgram.isLinked())
    {
        std::cout << "Shader Linking Error" << std::endl;
        std::cout << "" << basicProgram.log() << std::endl << std::endl;
    }
    basicProgram.bindFragmentOutput("fragColor");
    vShader.free();
    fShader.free();
}
