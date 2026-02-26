#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Quaternion.h"

class Camera{
  public:

    // position and speed
    glm::vec3 camPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    float camSpeed = 10.0f;
    Quaternion orientation = Quaternion(1.0f,0.0f,0.0f,0.0f);

    double mouseX;
    double mouseY;

    double lastMouseX =  800.0 / 2.0;
    double lastMouseY =  600.0 / 2.0;

    bool mousePressed = true;
    float sensitivity = 0.001f;


    // orientation based on the basis vectors
    glm::vec3 iHat;
    glm::vec3 jHat;
    glm::vec3 kHat;

    //clamping rotation variables
    double currentPitch = 0.0;
    double currentYaw = 0.0;

    Camera(double x, double y, double z, double cameraSpeed){
      camPosition = glm::vec3(x,y,z);
      camSpeed = cameraSpeed;
      orientation = Quaternion(1.0f,0.0f,0.0f,0.0f);

      updateCam();
    }

    void updateCam(){

      // basis vectors that help to determine direction even after rotation
      glm::vec3 defaultForward = glm::vec3(0.0f, 0.0f, -1.0f);
      glm::vec3 defaultRight = glm::vec3(1.0f, 0.0f, 0.0f);
      glm::vec3 defaultUp = glm::vec3(0.0f, 1.0f, 0.0f);
      
      // Rotate the default vectors by camera orientation
      kHat= orientation.rotate(defaultForward);
      iHat= orientation.rotate(defaultRight);
      jHat= orientation.rotate(defaultUp);

    }

    void setCam(double x, double y, double z, double cameraSpeed){

      //Initialize camera based on position and speed
      camPosition = glm::vec3(x, y, z);
      camSpeed = cameraSpeed;
      orientation = Quaternion(1.0f,0.0f,0.0f,0.0f);

      updateCam();
    }

    void rotate(double pitch, double yaw){

      currentPitch += pitch;
      currentYaw   += yaw;

      if (currentPitch > 89.0){currentPitch = 89.0;}
      if (currentPitch < -89.0){currentPitch = -89.0;}

      if (currentYaw > 359.0)  currentYaw -= 359.0;
      if (currentYaw < -359.0) currentYaw += 359.0;
    
      //create the rotations as quaternions
      Quaternion pitchQ = Quaternion::quaternionRotation(glm::vec3(1.0f, 0.0f, 0.0f), currentPitch);
      Quaternion yawQ   = Quaternion::quaternionRotation(glm::vec3(0.0f, 1.0f, 0.0f), currentYaw);
      
      //apply yaw then pitch
      orientation = yawQ * pitchQ;
      orientation.normalize();
      
      updateCam();
    }


    void moveZ(float amount){
      camPosition += kHat * amount;
    }

    void moveX(float amount) {
      camPosition += iHat * amount;
    }

    void moveY(float amount) {
      camPosition += jHat * amount;
    }

    glm::vec3 get_kHat(){return kHat;}
    glm::vec3 get_iHat(){return iHat;}
    glm::vec3 get_jHat(){return jHat;}

    glm::vec3 getPosition() {
      return glm::vec3(camPosition.x, camPosition.y, camPosition.z);
    }

    void set_position(double x, double y, double z) {
      camPosition.x = x;
      camPosition.y = y;
      camPosition.z = z;
    } 
    
    void CameraSystem(GLFWwindow* window){
      
      glfwGetCursorPos(window, &mouseX, &mouseY);

      if (mousePressed)
      {
          lastMouseX = mouseX;
          lastMouseY = mouseY;
          mousePressed = false;
      }

      float deltaX = (mouseX - lastMouseX) * sensitivity;
      float deltaY = (mouseY - lastMouseY) * sensitivity;

      lastMouseX = mouseX;
      lastMouseY = mouseY;

      rotate(-deltaY * 50.0f, -deltaX * 50.0f);
    }


void MoveCamera(GLFWwindow* window, float deltaTime){
    float moveSpeed = 300.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveZ(moveSpeed);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveZ(-moveSpeed);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveX(-moveSpeed);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveX(moveSpeed);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        moveY(moveSpeed);

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        moveY(-moveSpeed);
  }
}; 