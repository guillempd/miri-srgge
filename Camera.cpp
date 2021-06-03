#include "Camera.h"
#include "Application.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <iostream>

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::init()
{
    position = glm::vec3(0.0f, 0.0f, 3.0f);
    forward = glm::vec3(0.0f, 0.0f, -1.0f);
    right = glm::vec3(1.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    lookDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    theta = glm::half_pi<float>();
    phi = 0.0f;
    speed = 0.005f;
    updateViewMatrix();
}

void Camera::update(float deltaTime)
{
    if (Application::instance().getKey('w')) moveForward(1.0f, deltaTime);
    if (Application::instance().getKey('s')) moveForward(-1.0f, deltaTime);
    if (Application::instance().getKey('a')) moveRight(-1.0f, deltaTime);
    if (Application::instance().getKey('d')) moveRight(1.0f, deltaTime);
    if (Application::instance().getKey('q')) moveUp(-1.0f, deltaTime);
    if (Application::instance().getKey('e')) moveUp(1.0f, deltaTime);
    updateViewMatrix();
}

void Camera::resizeCameraViewport(int width, int height)
{
    projection = glm::perspective(60.f / 180.f * glm::pi<float>(), float(width) / float(height), 0.01f, 100.0f);
}

void Camera::rotateCamera(float xRotation, float yRotation)
{
    theta += xRotation;
    phi += yRotation;
    phi = glm::clamp(phi, -(glm::half_pi<float>() - 0.1f), glm::half_pi<float>() - 0.1f);
    updateLookDirection();
}

void Camera::updateLookDirection()
{
    lookDirection = glm::vec3(glm::cos(phi) * glm::cos(theta) ,glm::sin(phi), -glm::cos(phi) * glm::sin(theta));
    forward = glm::normalize(glm::vec3(lookDirection.x, 0.0f, lookDirection.z));
    right = glm::cross(forward, up);
    updateViewMatrix();
}

void Camera::moveForward(float input, float deltaTime)
{
    position += input * forward * speed * deltaTime;
}

void Camera::moveUp(float input, float deltaTime)
{
    position += input * up * speed * deltaTime;
}

void Camera::moveRight(float input, float deltaTime)
{
    position += input * right * speed * deltaTime;
}

// TODO: Implement this changing fov
void Camera::zoomCamera(float distDelta)
{

}

void Camera::updateViewMatrix()
{
    view = glm::lookAt(position, position + lookDirection, up);
}

glm::mat4 &Camera::getProjectionMatrix()
{
    return projection;
}

glm::mat4 &Camera::getViewMatrix()
{
    return view;
}
