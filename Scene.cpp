#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Scene.h"
#include "PLYReader.h"

Scene::Scene()
{
    mesh = NULL;
}

Scene::~Scene()
{
    if (mesh != NULL)
        delete mesh;
}

void Scene::init()
{
    initShaders();
    mesh = new TriangleMesh();
    mesh->buildCube();
    mesh->sendToOpenGL(basicProgram);
    currentTime = 0.0f;

    camera.init();

    bPolygonFill = true;
}

bool Scene::loadMesh(const char *filename)
{
    PLYReader reader;

    mesh->free();
    bool bSuccess = reader.readMesh(filename, *mesh);
    if (bSuccess)
        mesh->sendToOpenGL(basicProgram);

    return bSuccess;
}

void Scene::update(int deltaTime)
{
    currentTime += deltaTime;
    camera.update(deltaTime);
}

void Scene::render(int n)
{
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
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                render(i, j, viewMatrix);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_POLYGON_OFFSET_FILL);
        basicProgram.setUniform4f("color", 0.0f, 0.0f, 0.0f, 1.0f);
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            render(i, j, viewMatrix);            
}

void Scene::render(int i, int j, const glm::mat4 &viewMatrix)
{
    glm::mat4 modelMatrix(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(2*i, 0, -2*j));
    basicProgram.setUniformMatrix4f("model", modelMatrix);

    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(viewMatrix * modelMatrix));
    basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
    
    mesh->render();
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
