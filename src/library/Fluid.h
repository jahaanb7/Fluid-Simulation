#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

#include "Particle.h"

class Fluid : public Particle{
  public:

    // Properties of fluids
    float viscosity;
    float volume;
    float mass;

    float h;

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
  float SpikyKernel(Particle j){

    glm::vec3 distanceOfParticles = position - j.position;
    float r = glm::length(distanceOfParticles);

    glm::vec3 unitVector = distanceOfParticles / r;

    if(r < h){
      float scalar = -(45.0f / (M_PI * pow(h, 6.0f))) * (pow((h - r), 2.0f));
      glm::vec3 kernel =  scalar * unitVector;
    }
  }

  float getDensity(){

  }

  float getPressure(){
    
  }

};