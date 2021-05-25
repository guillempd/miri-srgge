#ifndef _CAMERA_INCLUDE
#define _CAMERA_INCLUDE

#include <glm/glm.hpp>

// Camera contains the properies of the camera the scene is using
// It is responsible for computing the associated GL matrices

class Camera
{

public:
    Camera();
    ~Camera();
    void init();
    void update(float deltaTime);
    void resizeCameraViewport(int width, int height);
    void rotateCamera(float xRotation, float yRotation);
    void zoomCamera(float distDelta);
    glm::mat4 &getProjectionMatrix();
    glm::mat4 &getViewMatrix();
    const glm::vec3 &getPosition() {return position;}
private:
    void moveForward(float input, float deltaTime);
    void moveRight(float input, float deltaTime);
    void moveUp(float input, float deltaTime);
    void updateViewMatrix();
    void updateLookDirection();
private:
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up; // TODO: make this static
    glm::vec3 lookDirection;
    float theta;
    float phi;
    float speed;
};

#endif // _CAMERA_INCLUDE
