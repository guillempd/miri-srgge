#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include "Camera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"

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
    void render(int n);

    Camera &getCamera();

    void switchPolygonMode();

private:
    void initShaders();
    void computeModelViewMatrix();

    void render(int i, int j, const glm::mat4 &cameraModelView);

private:
    Camera camera;
    std::vector<TriangleMesh*> meshes;
    ShaderProgram basicProgram;
    float currentTime;
    std::vector<std::vector<unsigned char>> tilemap;
    std::vector<unsigned char> tile;
    bool bPolygonFill;
};

#endif // _SCENE_INCLUDE
