#include "Scene.h"
#include "PLYReader.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "imgui.h"

#include <iostream>
#include <fstream>
#include <queue>
#include <string>

Scene::Scene()
{

}

Scene::~Scene()
{

}

void Scene::init()
{
    initShaders();

    currentTime = 0.0f;

    camera.init();

    bPolygonFill = true;

    wall.buildCube();
    wall.sendToOpenGL(basicProgram);
}

bool Scene::loadScene(const char *filename)
{
    std::ifstream fin(filename, std::ios::in);
    if (!fin.is_open()) return false;

    std::vector<int> tile(256, -1);

    // Read & Load models
    int n;
    fin >> n;
    for (int i = 0; i < n; ++i)
    {
        unsigned char c;
        std::string modelDirectory;
        fin >> c >> modelDirectory;
        models.emplace_back();
        loadModel(modelDirectory, models[i]);
        tile[c] = i;
    }

    // Read & Store walls and statues
    int w, h;
    fin >> w >> h;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            unsigned char c;
            fin >> c;
            if (c == 'x') {
                walls.emplace_back(x, y);
            }
            else if (tile[c] >= 0) {
                Statue statue = {models[tile[c]], glm::ivec2(x, y)};
                statues.push_back(statue);
            }
        }
    }
    return true;
}

void Scene::loadModel(const std::string &modelDirectory, MeshLods &model)
{
    for (int i = 0; i < 4; ++i) {
        TriangleMesh &lod = model.lods[i];
        std::string meshFilename = modelDirectory + "/" + std::to_string(i) + ".ply";
        PLYReader::readMesh(meshFilename, lod);
        lod.sendToOpenGL(basicProgram);
    }
}

void Scene::update(int deltaTime)
{
    currentTime += deltaTime;
    camera.update(deltaTime);
}

void Scene::render(bool debugColors)
{
    if (ImGui::Begin("Some settings")) {
        ImGui::SliderFloat("TPS", &TPS, 1e6, 1e9, "%g", ImGuiSliderFlags_Logarithmic);
    }
    ImGui::End();

    const glm::mat4 &view = camera.getViewMatrix();
    const glm::mat4 &projection = camera.getProjectionMatrix();

    basicProgram.use();
    basicProgram.setUniformMatrix4f("view", view);
    basicProgram.setUniformMatrix4f("projection", projection);
    basicProgram.setUniform1i("bLighting", bPolygonFill ? 1 : 0);
    basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    renderWalls();
    renderStatues(debugColors);         
}

float Scene::distanceToCamera(const glm::ivec2 &statuePosition) const
{
    return glm::length(camera.getPosition() - glm::vec3(statuePosition.x, 0.0f, statuePosition.y));
}

float Scene::deltaBenefit(int lod, int index) const
{
    const Statue &statue = statues[index];
    const AABB &statueAABB = statue.meshLods.lods[0].aabb;
    float D = distanceToCamera(statue.position);
    float d = 1.1f * glm::length(statueAABB.max - statueAABB.min);
    return d / (D * glm::pow(2, lod));
}

float Scene::deltaCost(int lod, int index) const
{
    const Statue &statue = statues[index];
    const MeshLods &meshLods = statue.meshLods;
    int new_triangles = meshLods.lods[lod].triangles.size();
    int previous_triangles = meshLods.lods[lod-1].triangles.size();
    return new_triangles - previous_triangles;
}

Assignment Scene::nextAssignment(int index, int lod) const
{
    return {index, lod + 1, deltaBenefit(lod + 1, index), deltaCost(lod + 1, index)};
}

// TODO: Improve by starting with the same lods as previous frame
void Scene::renderStatues(bool debugColors)
{
    int n = statues.size();
    
    // Initially assign all statues the lowest lod
    std::vector<int> statuesLod(n, 0);

    // Greedy algorithm
    using PriorityQueue = std::priority_queue<Assignment,std::vector<Assignment>,AssignmentPriority>;
    PriorityQueue improvements;
    for (int i = 0; i < n; ++i) {
        improvements.push(nextAssignment(i, 0));
    }

    float cost = 0.0f;
    float max_cost = TPS/FPS;
    while (cost < max_cost && !improvements.empty()) {
        Assignment assignment = improvements.top();
        improvements.pop();
        if (cost + assignment.cost <= max_cost) {
            cost += assignment.cost;
            statuesLod[assignment.index] = assignment.lod;
            if (assignment.lod < 3) { 
                improvements.push(nextAssignment(assignment.index, assignment.lod));
            }
        }
    }

    // Render final assignments
    for (int i = 0; i < n; ++i) {
        const Statue &statue = statues[i];
        int lod = statuesLod[i];
        if (debugColors) {
            switch(lod) {
                case 0:
                    basicProgram.setUniform4f("color", 1.0f, 0.0f, 0.0f, 1.0f);
                    break;
                case 1:
                    basicProgram.setUniform4f("color", 1.0f, 0.5, 0.0f, 0.0f);
                    break;
                case 2:
                    basicProgram.setUniform4f("color", 1.0f, 1.0f, 0.0f, 0.0f);
                    break; 
                case 3:
                    basicProgram.setUniform4f("color", 0.5f, 1.0f, 0.0f, 1.0f);
                    break;
            }
        }
        render(statue.meshLods.lods[lod], statue.position);
    }
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
