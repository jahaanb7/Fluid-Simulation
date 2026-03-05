#include <iostream>
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "library/Physics.h"
#include "library/Particle.h"
#include "library/Camera.h"


// Global Variables:

//screen dimensions
int const WIDTH = 800;
int const HEIGHT = 800;
int const DEPTH = 800;

// Variables for projection matrix
const float fov = glm::radians(60.0f);
const float aspectRatio = WIDTH / HEIGHT;
const float nearPlane =  0.1f;
const float farPlane = 5000.0f;

// Disable mouse movement
bool mouseLock = false;

// Variables for particle grid arrangement
const int rows = 3;
const int cols = 10;
const int zRange = 10;
const float spacing = 5;

double lastFrame = 0.0f; 

//Initialize camera
Camera cam(0.0f, 0.0f, 1000.0f, 10.0f, mouseLock);

// resize the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

int main(){

  //initialize GLFW for rendering window
  if(!glfwInit()){
    std::cout<<"Failed to Initialize GLFW"<<std::endl;
    return -1;
  }

  // create GLFWwindow
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Fluid Simulation", NULL, NULL);
  if (window == NULL){
    std::cout<<"Failed to create GLFW window"<<std::endl;
    glfwTerminate();
    return -1;
  }
  
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // Mac does not support this Retina displays
  int fbWidth, fbHeight;
  glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

  glViewport(0, 0, fbWidth, fbHeight);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  
  // Depth Test (DepthBuffer)
  glEnable(GL_DEPTH_TEST);

  // Enable positional lighting that
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  Particle particle = Particle(glm::vec3(-350.0f, 200.0f, 0.0f), glm::vec3(3.0f, 40.0f, 0.0f), 5);

  drawParticleGrid3D(rows, cols, zRange, spacing, particle);

  // Render loop: handles user events and inputs
  while(!glfwWindowShouldClose(window)){

    // deltaTime - frames per second (0.000167)
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastFrame;
    lastFrame = currentTime;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Perspective Projection (Perspective Matrix)
    glm::mat4 projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(projection));

    // View pipeline (Camera, ViewModel Matrix)
    cam.CameraSystem(window);

    glm::vec3 camPosition = cam.getPosition();
    glm::vec3 forward = cam.get_kHat();
    glm::vec3 up = cam.get_jHat();
    
    cam.MoveCamera(window, deltaTime);

    glm::mat4 view = glm::lookAt(camPosition, camPosition + forward, up);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(view));
    
    // set light as positional and set (x,y,z) coordinates
    GLfloat light[] = {camPosition.x, camPosition.y, camPosition.z, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light);

    update(deltaTime, WIDTH, HEIGHT, DEPTH);
    drawBoundaryBox(WIDTH, HEIGHT, DEPTH/2.0f);

    glfwSwapBuffers(window);
    glfwPollEvents();    
  }

  glfwTerminate();
  return 0;
}