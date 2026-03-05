#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

#include "Particle.h"

class Fluid : public Particle{
  public:

    // Properties of fluids
    const float viscosity = 0.1f;
    const float restDensity = 1000.0f;
    const float stiffness = 200.0f;

    //smoothing radius for kernel
    const float h = 12.0f;

    glm::vec3 Force;
    glm::vec3 Color;

    
  /*
  SPH (Smooth Particle Hydrodynamics)

  r --> distance between two particles
  h --> the smoothing length (influences the particles nieghbors)

  At r = 0 (same position) --> maximum influence
  At r = h (edge of radius) --> influence is zero
  */ 


  // calculates for density force
  float Poly6Kernel(Particle j){

    glm::vec3 distanceOfParticles = position - j.position;
    float r = glm::length(distanceOfParticles);

    if(r < h){
      float kernel = (315.0f / (64.0f * M_PI * (pow(h, 9)))) * (pow(((h*h) - (r*r)), 3));
      return kernel;
    }

    else{
      return 0.0f;
    }
  }

  // calculates for pressure force
  glm::vec3 SpikyKernel(Particle j){

    glm::vec3 distanceOfParticles = position - j.position;
    float r = glm::length(distanceOfParticles);

    glm::vec3 unitVector = distanceOfParticles / r;

    if(r < h){
      float scalar = -(45.0f / (M_PI * pow(h, 6.0f))) * (pow((h - r), 2.0f));
      glm::vec3 kernel = scalar * unitVector;

      return kernel;
    }
  }

  // calculates for viscosity
  float LaplacianKernel(Particle j){

    glm::vec3 distanceOfParticles = position - j.position;
    float r = glm::length(distanceOfParticles);
    
    float kernel = (45.0f / (M_PI * pow(h, 6))) * (h - r);
    
    return kernel;
  }

  float getDensity(){

  }

  float getPressure(){

  }

};