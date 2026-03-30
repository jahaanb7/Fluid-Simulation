#include <iostream>
#include <cmath>
#include <algorithm>
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifdef _OPENMP
#include <omp.h>
#endif

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
const float nearPlane =  0.1f;
const float farPlane = 5000.0f;

// Disable mouse movement
bool mouseLock = true;

// Demo / interactivity state
bool paused = false;
float timeScale = 1.0f;
float injectionStrength = 4.0f;
float injectionRadius = 0.45f;

// Variables for particle grid arrangement
const int rows = 15;
const int cols = 15;
const int zRange = 15;
const float spacing = 0.08f;

const float radius = spacing * 0.5f;
const float camSpeed = 0.5f;

double lastFrame = 0.0f; 

//Initialize camera
Camera cam(0.0f, 0.0f,  6.0f, camSpeed);

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
  
  // Keep the real cursor confined to the app window.
  // We render a software cursor when hovering the ImGui GUI.
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPos(window, SCREENWIDTH / 2.0, SCREENHEIGHT / 2.0);
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
  glm::vec3 fluidCenter = particle.position;
  Particle seedParticle = particle;

  auto resetSimulation = [&](){
    drawParticleGrid3D(rows, cols, zRange, spacing, seedParticle);
    fluid.initParticles();
  };

  auto applyRadialImpulse = [&](const glm::vec3& point, float radius, float strength){
    for(auto& p : fluid.particles){
      if(p.isBoundary) continue;

      glm::vec3 diff = p.position - point;
      float dist = glm::length(diff);
      if(dist <= 0.00001f || dist > radius) continue;

      float t = 1.0f - (dist / radius); // 0..1 falloff
      glm::vec3 dir = diff / dist;      // outward

      p.velocity += dir * (strength * t);
      // Add a bit of upward buoyancy for more visible motion.
      p.velocity += glm::vec3(0.0f, strength * 0.25f * t, 0.0f);
    }
  };

  resetSimulation();

  bool mDownPrev = false;
  bool spaceDownPrev = false;
  bool rDownPrev = false;
  bool iDownPrev = false;
  bool lmbDownPrev = false;

  // Render loop: handles user events and inputs
  while(!glfwWindowShouldClose(window)){

    // deltaTime - frames per second (0.000167)
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastFrame;
    deltaTime = std::min(deltaTime, 0.016);  // cap at 16ms (60fps)
    lastFrame = currentTime;

    const double effectiveDeltaTime = paused ? 0.0 : (deltaTime * timeScale);

    int fbWidth = 1, fbHeight = 1;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    float aspectRatio = (fbHeight != 0) ? ((float)fbWidth / (float)fbHeight) : 1.0f;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Perspective Projection (Perspective Matrix)
    glm::mat4 projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(projection));

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
      
    ImGui::Begin("Fluid Controls");
      ImGui::SliderFloat("Target Density", &fluid.targetDensity, 0.0f, 500.0f);
      ImGui::SliderFloat("Stiffness", &fluid.stiffness, 0.0f, 100.0f);
      ImGui::SliderFloat("Viscosity", &fluid.viscosity, 0.0f, 1.0f);
      ImGui::SliderFloat("Gravity", &fluid.gravity, -10.0f, 10.0f);

      ImGui::Separator();
      ImGui::Checkbox("Pause (Space)", &paused);
      ImGui::SliderFloat("Time Scale", &timeScale, 0.0f, 2.0f);

      ImGui::Separator();
      ImGui::SliderFloat("Impulse Strength", &injectionStrength, 0.0f, 20.0f);
      ImGui::SliderFloat("Impulse Radius", &injectionRadius, 0.05f, 2.0f);

      if(ImGui::Button("Inject Pulse (I)")){
        applyRadialImpulse(fluidCenter, injectionRadius, injectionStrength);
      }
      ImGui::SameLine();
      if(ImGui::Button("Reset (R)")){
        resetSimulation();
      }

      ImGui::Separator();
      ImGui::TextUnformatted("Tip: Left click to stir (when not over UI).");
      ImGui::TextUnformatted("M: toggle mouse camera control.");
    ImGui::End();

    ImGuiIO& io = ImGui::GetIO();
    bool wantCaptureMouse = io.WantCaptureMouse;

    // Rising-edge keyboard handling (avoids toggling every frame).
    bool mDown = glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS;
    if(mDown && !mDownPrev){
      mouseLock = !mouseLock;
      cam.mousePressed = true; // prevents sudden camera jumps after unlocking
    }
    mDownPrev = mDown;

    bool spaceDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if(spaceDown && !spaceDownPrev){
      paused = !paused;
    }
    spaceDownPrev = spaceDown;

    bool rDown = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if(rDown && !rDownPrev){
      resetSimulation();
    }
    rDownPrev = rDown;

    bool iDown = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
    if(iDown && !iDownPrev){
      applyRadialImpulse(fluidCenter, injectionRadius, injectionStrength);
    }
    iDownPrev = iDown;

    // View pipeline (Camera, ViewModel Matrix)
    cam.setMouseLock(mouseLock);
    cam.CameraSystem(window);

    glm::vec3 camPosition = cam.getPosition();
    glm::vec3 forward = cam.get_kHat();
    glm::vec3 up = cam.get_jHat();
    
    cam.MoveCamera(window, (float)(deltaTime * 5.0));

    glm::mat4 view = glm::lookAt(camPosition, camPosition + forward, up);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(view));
    
    // set light as positional and set (x,y,z) coordinates
    GLfloat light[] = {camPosition.x, camPosition.y, camPosition.z, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light);

    // Software cursor: the real cursor is disabled to keep it inside the window.
    // Show a simple cursor only when the ImGui GUI wants mouse input.
    if(wantCaptureMouse){
      ImDrawList* drawList = ImGui::GetForegroundDrawList();
      ImVec2 p = io.MousePos;
      const float r = 4.0f;
      drawList->AddCircleFilled(p, r, IM_COL32(255, 255, 255, 255));
      drawList->AddCircle(p, r + 2.0f, IM_COL32(0, 0, 0, 200));
      drawList->AddLine(ImVec2(p.x - 7.0f, p.y), ImVec2(p.x + 7.0f, p.y), IM_COL32(255,255,255,200));
      drawList->AddLine(ImVec2(p.x, p.y - 7.0f), ImVec2(p.x, p.y + 7.0f), IM_COL32(255,255,255,200));
    }

    // Click-to-stir (impulse) when not interacting with ImGui.
    bool lmbDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if(lmbDown && !lmbDownPrev && !wantCaptureMouse && !paused){
      double mx = 0.0, my = 0.0;
      glfwGetCursorPos(window, &mx, &my);

      // Convert window coordinates to NDC [-1, 1].
      float ndcX = (2.0f * (float)mx) / (float)fbWidth - 1.0f;
      float ndcY = 1.0f - (2.0f * (float)my) / (float)fbHeight;

      // Unproject two points to get a world-space ray.
      glm::mat4 invVP = glm::inverse(projection * view);
      glm::vec4 nearClip(ndcX, ndcY, -1.0f, 1.0f);
      glm::vec4 farClip(ndcX, ndcY, 1.0f, 1.0f);

      glm::vec4 nearWorld = invVP * nearClip;
      nearWorld /= nearWorld.w;
      glm::vec4 farWorld = invVP * farClip;
      farWorld /= farWorld.w;

      glm::vec3 rayOrigin = camPosition;
      glm::vec3 rayDir = glm::normalize(glm::vec3(farWorld - nearWorld));

      // Intersect with a horizontal plane through the initial fluid center.
      float planeY = fluidCenter.y;
      float denom = rayDir.y;
      if(std::abs(denom) > 1e-6f){
        float t = (planeY - rayOrigin.y) / denom;
        if(t > 0.0f){
          glm::vec3 hitPoint = rayOrigin + t * rayDir;
          applyRadialImpulse(hitPoint, injectionRadius, injectionStrength);
        }
      }
    }
    lmbDownPrev = lmbDown;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    update((float)effectiveDeltaTime, WIDTH, HEIGHT, DEPTH);
    drawBoundaryBox(WIDTH, HEIGHT, DEPTH);

    glfwSwapBuffers(window);
    glfwPollEvents();    
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
  return 0;
}