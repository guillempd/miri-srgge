#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include "Camera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"
#include "TimeCritical.h"

#include <glm/glm.hpp>

#include <vector>

// Scene contains all the entities of our game.
// It is responsible for updating and render them.

class Scene
{

public:
    Scene();
    ~Scene();

    void init();
    bool loadMesh(const char *filename);
    bool loadScene(const char * filename);
    void update(int deltaTime);
    void render();

    Camera &getCamera();

    void switchPolygonMode();

private:
    void initShaders();

    void renderWalls();
    void renderStatues();
    void render(const TriangleMesh &mesh, const glm::ivec2 &gridCoordinates);

private:
    Camera camera;
    std::vector<TriangleMesh*> meshes;
    ShaderProgram basicProgram;
    float currentTime;
    std::vector<std::vector<unsigned char>> tilemap;
    std::vector<unsigned char> tile;
    bool bPolygonFill;

    TriangleMesh wall;
    std::vector<MeshLods> models; // models
    std::vector<Statue> statues; // statues to render
    std::vector<glm::ivec2> walls; // walls to render
};

#endif // _SCENE_INCLUDE
