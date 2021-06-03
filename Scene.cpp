#include "Scene.h"
#include "PLYReader.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "imgui.h"

#include <iostream>
#include <fstream>
#include <sstream>
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
    std::vector<int> modelIndex;
    if (!loadModels(filename, modelIndex)) return false;
    if (!loadFloorPlan(filename, modelIndex)) return false;
    if (!loadVisibility(filename)) return false;
    return true;
}

bool Scene::loadModels(const char *filename, std::vector<int> &modelIndex)
{
    std::string models_extension = ".m";
    std::ifstream fin(filename + models_extension);
    if (!fin.is_open()) return false;

    modelIndex = std::vector<int>(256, -1);

    int n;
    fin >> n;
    for (int i = 0; i < n; ++i) {
        unsigned char c;
        std::string modelDirectory;
        fin >> c >> modelDirectory;
        models.emplace_back();
        loadModel(modelDirectory, models[i]);
        modelIndex[c] = i;
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

bool Scene::loadFloorPlan(const char *filename, std::vector<int> &modelIndex)
{
    std::string floor_plan_extension = ".tm";
    std::ifstream fin(filename + floor_plan_extension);
    if (!fin.is_open()) return false;

    fin >> width >> height;
    floorPlan = std::vector<std::vector<int>> (width, std::vector<int>(height, -1));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char c;
            fin >> c;
            if (c == 'x') walls.emplace_back(x, y);
            else if (modelIndex[c] >= 0) floorPlan[x][y] = modelIndex[c];
        }
    }
    return true;
}

bool Scene::loadVisibility(const char *filename)
{
    std::string visibility_extension = ".v";
    std::ifstream fin(filename + visibility_extension);
    if (!fin.is_open()) return false;

    visibleFrom = std::vector<std::vector<std::vector<glm::ivec2>>> (width, std::vector<std::vector<glm::ivec2>>(height));
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            std::string line;
            std::getline(fin, line);
            std::istringstream sin(line);

            int x_0, y_0;
            sin >> x_0 >> y_0;
            int x_vis, y_vis;
            while (sin >> x_vis >> y_vis) visibleFrom[x_0][y_0].push_back({x_vis, y_vis});
        }
    }
    return true;
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
    const Statue &statue = PVS[index];
    const AABB &statueAABB = statue.meshLods.lods[0].aabb;
    float D = distanceToCamera(statue.position);
    float d = 1.1f * glm::length(statueAABB.max - statueAABB.min);
    return d / (D * glm::pow(2, lod));
}

float Scene::deltaCost(int lod, int index) const
{
    const Statue &statue = PVS[index];
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
    recomputePVS();

    int n = PVS.size();
    
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
        const Statue &statue = PVS[i];
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
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(gridCoordinates.x + 0.5f, +0.5f, gridCoordinates.y + 0.5f));
    basicProgram.setUniformMatrix4f("model", model);

    const glm::mat4 &view = camera.getViewMatrix();
    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);

    mesh.render();
}

void Scene::recomputePVS()
{
    PVS.clear();

    glm::vec3 cameraPosition = camera.getPosition();
    glm::ivec2 gridPosition = glm::ivec2(cameraPosition.x, cameraPosition.z);
    gridPosition = glm::clamp(gridPosition, glm::ivec2(0, 0), glm::ivec2(width-1, height-1));

    for (auto statueGridPosition : visibleFrom[gridPosition.x][gridPosition.y]) {
        int modelIndex = floorPlan[statueGridPosition.x][statueGridPosition.y];
        PVS.push_back({models[modelIndex], statueGridPosition});
    }
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
