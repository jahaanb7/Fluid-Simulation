#include <iostream>
#include <GLFW/glfw3.h>
#include <GLAD/glad.h>

float const WIDTH = 800.0f;
float const HEIGHT = 800.0f;

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

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // create GLFWwindow
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Fluid Simulation", NULL, NULL);

  if (window == NULL){
    std::cout<<"Failed to create GLFW window"<<std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  glViewport(0, 0, WIDTH, HEIGHT);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  
  // Render loop: handles user events and inputs
  while(!glfwWindowShouldClose(window)){
    glfwSwapBuffers(window);
    glfwPollEvents();    
  }

  glfwTerminate();
  return 0;
}