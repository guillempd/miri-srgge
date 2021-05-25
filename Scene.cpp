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

    wall.buildCube();
    wall.sendToOpenGL(basicProgram);
}

// TODO: Load scene and fill models, statues and walls
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
            if (tilemap[x][y] == 'x') {
                walls.emplace_back(x, y);
            }
        }
    }
    return true;
}

void Scene::update(int deltaTime)
{
    currentTime += deltaTime;
    camera.update(deltaTime);
}

void Scene::render()
{
    const glm::mat4 &view = camera.getViewMatrix();
    const glm::mat4 &projection = camera.getProjectionMatrix();

    basicProgram.use();
    basicProgram.setUniformMatrix4f("view", view);
    basicProgram.setUniformMatrix4f("projection", projection);
    basicProgram.setUniform1i("bLighting", bPolygonFill ? 1 : 0);
    basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    renderWalls();
    renderStatues();
    // basicProgram.setUniform1i("bLighting", bPolygonFill ? 1 : 0);
    // if (bPolygonFill)
    // {
    //     basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
    //     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // }
    // else
    // {
    //     basicProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
    //     glEnable(GL_POLYGON_OFFSET_FILL);
    //     glPolygonOffset(0.5f, 1.0f);
    //     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //     for (int x = 0; x < w; ++x)
    //         for (int y = 0; y < h; ++y)
    //             render(x, y, viewMatrix);
    //     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //     glDisable(GL_POLYGON_OFFSET_FILL);
    //     basicProgram.setUniform4f("color", 0.0f, 0.0f, 0.0f, 1.0f);
    // }          
}

// void Scene::render(int x, int y, const glm::mat4 &viewMatrix)
// {
//     unsigned char b = tilemap[x][y];
//     unsigned char i = tile[b];
//     if (i >= 0 && b != '.')
//     {
        

        
//     }
//     // TODO: Render floor in any case
// }

void Scene::renderStatues()
{
    // TODO: Implement time-critical rendering
}

void Scene::renderWalls()
{
    for (const glm::ivec2 &gridCoordinates : walls)
        render(wall, gridCoordinates);
}

void Scene::render(const TriangleMesh &mesh, const glm::ivec2 &gridCoordinates)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(gridCoordinates.x, 0, gridCoordinates.y));
    basicProgram.setUniformMatrix4f("model", model);

    const glm::mat4 &view = camera.getViewMatrix();
    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);

    mesh.render();
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
