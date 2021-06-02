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
    bool loadScene(const char * filename);
    void update(int deltaTime);
    void render(bool debugColors);

    Camera &getCamera();

    void switchPolygonMode();

private:
    void initShaders();
    void loadModel(const std::string &modelDirectory, MeshLods &model);

    void renderWalls();
    void renderStatues(bool debugColors);
    void render(const TriangleMesh &mesh, const glm::ivec2 &gridCoordinates);

    float distanceToCamera(const glm::ivec2 &gridCoordinates) const;
    float deltaCost(int lod, int index) const;
    float deltaBenefit(int lod, int index) const;
    Assignment nextAssignment(int lod, int index) const;

    // Visibility computation
    void recomputePVS();

private:
    Camera camera;
    ShaderProgram basicProgram;
    float currentTime;
    bool bPolygonFill;

    int width;
    int height;

    TriangleMesh wall;
    std::vector<MeshLods> models; // models
    std::vector<glm::ivec2> walls; // walls to render

    // TODO: Actually measure TPS
    float TPS = 1e6;
    float FPS = 60.0f;

    // Visibility data structure
    // TODO: Initialize
    std::vector<Statue> PVS;
    std::vector<std::vector<std::vector<glm::ivec2>>> visibility; // visibility[x][y] is a list of the positions visible from (x,y), only store those with statues (walls are always rendered)
    std::vector<std::vector<int>> map; // map[x][y] is the index to the model occupying position (x,y) TODO: maybe change by char

};

#endif // _SCENE_INCLUDE
