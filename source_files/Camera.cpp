#include "Camera.hpp"
#include <GLFW\glfw3.h>
glm::vec3 worldUp;
namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        //Subtracting the camera position vector from the scene's origin vector results in the direction vector we want


        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, cameraUp));
        //RightDirection = x axis
        //cross product between y axis (cameraUp) and z axis (cameraFront) results in a vector perpendicular to those two, which is x axis
        
        worldUp = cameraUp;

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        if (direction == MOVE_BACKWARD)
        {
            this->cameraPosition -= speed * this->cameraFrontDirection;
        }

        if (direction == MOVE_FORWARD)
        {
            this->cameraPosition += speed * this->cameraFrontDirection;
        }

        if (direction == MOVE_LEFT)
        {
            this->cameraPosition -= speed * this->cameraRightDirection;
        }

        if (direction == MOVE_RIGHT)
        {
            this->cameraPosition += speed * this->cameraRightDirection;
        }

        if (direction == MOVE_DOWN)
        {
            this->cameraPosition -= speed * this->cameraUpDirection;
        }

        if (direction == MOVE_UP)
        {
            this->cameraPosition += speed * this->cameraUpDirection;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO

        glm::vec3 newFrontDirection;

        newFrontDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFrontDirection.y = sin(glm::radians(pitch));
        newFrontDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        this->cameraFrontDirection = glm::normalize(newFrontDirection);


        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, worldUp));
        this->cameraUpDirection = glm::normalize(glm::cross(this->cameraRightDirection, this->cameraFrontDirection));
    }
}