#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include "Camera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"
#include "TimeCritical.h"

#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// Scene contains all the entities of our game.
// It is responsible for updating and render them.

class Scene
{

public:
    Scene();
    ~Scene();

    void init();
    bool loadScene(const std::string &filename);
    void update(int deltaTime);
    void render();

    Camera &getCamera();

private:
    void initShaders();

    bool loadModels(const std::string &filename, std::vector<int> &modelIndex);
    bool loadFloorPlan(const std::string &filename, std::vector<int> &modelIndex);
    bool loadVisibility(const std::string &filename);

    void loadModel(const std::string &modelDirectory, MeshLods &model);

    void renderWalls();
    void renderStatues();
    void render(const TriangleMesh &mesh, const glm::ivec2 &gridCoordinates);

    float distanceToCamera(const glm::ivec2 &gridCoordinates) const;
    float deltaCost(int lod, int index) const;
    float deltaBenefit(int lod, int index) const;
    Assignment nextAssignment(int lod, int index) const;

    void recomputePVS();

private:
    // Scene element
    Camera camera;
    TriangleMesh wall;
    std::vector<MeshLods> models; // models
    std::vector<glm::ivec2> walls; // walls to render
    ShaderProgram basicProgram;

    // Time critical rendering data
    // TODO: Actually measure TPS
    float TPS = 1e6;
    float FPS = 60.0f;

    // Visibility data
    // TODO: Initialize
    std::vector<Statue> PVS;
    std::vector<std::vector<std::vector<glm::ivec2>>> visibleFrom; // visibility[x][y] is a list of the positions visible from (x,y), only store those with statues (walls are always rendered)
    std::vector<std::vector<int>> floorPlan; // map[x][y] is the index to the model occupying position (x,y) TODO: maybe change by char

    // Other data
    float currentTime;
    int width;
    int height;
    bool debugColors;
};

#endif // _SCENE_INCLUDE
