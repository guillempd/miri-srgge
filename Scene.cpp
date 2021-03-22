#include <iostream>
#include <cmath>
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

    camera.init(2.0f);

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
}

void Scene::render(int n)
{
    glm::mat4 &cameraModelView = camera.getModelViewMatrix();
    basicProgram.use();
    basicProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());
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
                render(i, j, cameraModelView);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_POLYGON_OFFSET_FILL);
        basicProgram.setUniform4f("color", 0.0f, 0.0f, 0.0f, 1.0f);
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            render(i, j, cameraModelView);            
}

void Scene::render(int i, int j, const glm::mat4 &cameraModelView )
{
    glm::mat4 modelView = glm::translate(cameraModelView, glm::vec3(2*i, 0, -2*j));
    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(modelView));
    basicProgram.setUniformMatrix4f("modelview", modelView);
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

    vShader.initFromFile(VERTEX_SHADER, "shaders/basic.vert");
    if (!vShader.isCompiled())
    {
        cout << "Vertex Shader Error" << endl;
        cout << "" << vShader.log() << endl
             << endl;
    }
    fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.frag");
    if (!fShader.isCompiled())
    {
        cout << "Fragment Shader Error" << endl;
        cout << "" << fShader.log() << endl
             << endl;
    }
    basicProgram.init();
    basicProgram.addShader(vShader);
    basicProgram.addShader(fShader);
    basicProgram.link();
    if (!basicProgram.isLinked())
    {
        cout << "Shader Linking Error" << endl;
        cout << "" << basicProgram.log() << endl
             << endl;
    }
    basicProgram.bindFragmentOutput("outColor");
    vShader.free();
    fShader.free();
}
