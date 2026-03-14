#include <iostream>
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <omp.h>

#include "library/Physics.h"
#include "library/Particle.h"
#include "library/Camera.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

// Global Variables:

//screen dimensions
int const SCREENWIDTH = 800;
int const SCREENHEIGHT = 800;

float const WIDTH =  2.0f;
float const HEIGHT = 2.0f;
float const DEPTH =  2.0f;

// Variables for projection matrix
const float fov = glm::radians(60.0f);
const float aspectRatio = (float)SCREENWIDTH / (float)SCREENHEIGHT;
const float nearPlane =  0.1f;
const float farPlane = 5000.0f;

// Disable mouse movement
bool mouseLock = false;

// Variables for particle grid arrangement
const int rows = 15;
const int cols = 15;
const int zRange = 15;
const float spacing = 0.08f;

const float radius = spacing * 0.5f;
const float camSpeed = 0.5f;

double lastFrame = 0.0f; 

//Initialize camera
Camera cam(0.5f, 1.0f,  6.0f, camSpeed, mouseLock);

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
  GLFWwindow* window = glfwCreateWindow(SCREENWIDTH, SCREENHEIGHT, "Fluid Simulation", NULL, NULL);
  if (window == NULL){
    std::cout<<"Failed to create GLFW window"<<std::endl;
    glfwTerminate();
    return -1;
  }
  
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwMakeContextCurrent(window);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 120");
  ImGui::StyleColorsDark();

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
  glEnable(GL_NORMALIZE); 

  GLfloat ambient[] = { 0.20f, 0.22f, 0.28f, 1.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  Particle particle = Particle(glm::vec3(0.60f, 0.3f, 0.870f), glm::vec3(0.0f, 0.0f, 0.0f), radius);

  drawParticleGrid3D(rows, cols, zRange, spacing, particle);
  fluid.initParticles(); 

  // Render loop: handles user events and inputs
  while(!glfwWindowShouldClose(window)){

    // deltaTime - frames per second (0.000167)
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastFrame;
    deltaTime = std::min(deltaTime, 0.016);  // cap at 16ms (60fps)
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
    
    cam.MoveCamera(window, deltaTime*5);

    glm::mat4 view = glm::lookAt(camPosition, camPosition + forward, up);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(view));
    
    // set light as positional and set (x,y,z) coordinates
    GLfloat light[] = {camPosition.x, camPosition.y, camPosition.z, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light);

    update(deltaTime, WIDTH, HEIGHT, DEPTH);
    drawBoundaryBox(WIDTH, HEIGHT, DEPTH);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
      
    ImGui::Begin("Fluid Controls");
      ImGui::SliderFloat("Target Density", &fluid.targetDensity, 0.0f, 500.0f);
      ImGui::SliderFloat("Stiffness", &fluid.stiffness, 0.0f, 5.0f);
      ImGui::SliderFloat("Viscosity", &fluid.viscosity, 0.0f, 1.0f);
      ImGui::SliderFloat("Gravity", &fluid.gravity, 0.0f, 10.0f);
    ImGui::End();

    if(ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)){
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      mouseLock = true;
    } 
    
    else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      mouseLock = false;
    }

    cam.CameraSystem(window);
    cam.MoveCamera(window, deltaTime * 5);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();    
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
  return 0;
}