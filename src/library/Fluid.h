#include <iostream>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

#include "Particle.h"
#include "SpatialGrid.h"

class Fluid{
  
  public:

    std::vector<Particle> particles;

    // Properties of fluids (restDensity is the density of the fluid we want to eventually achieve)
    const float viscosity = 0.1f;
    const float restDensity = 4.5f;
    const float stiffness = 1.70f;

    //smoothing radius for kernel
    const float h = 12.0f;

    SpatialGrid spatialGrid{h};

  /*

  SPH (Smooth Particle Hydrodynamics)

  r --> distance between two particles
  h --> the smoothing length (influences the particles nieghbors)

  At r = 0 (same position) --> maximum influence
  At r = h (edge of radius) --> influence is zero

  Poly6    --> density    --> how much fluid is nearby (scalar)
  Spiky    --> pressure   --> push particles apart (vector)
  Laplacian --> viscosity --> smooth out velocity differences (scalar)

  */ 


  // calculates for density force
  float Poly6Kernel(Particle& i, Particle& j){

    float r = getDistanceParticles(i.position, j.position);

    if(r < h){
      float h9 = h*h*h*h*h*h*h*h*h;

      float hMinusr = ((h*h) - (r*r))*((h*h) - (r*r))*((h*h) - (r*r));

      float kernel = (315.0f/ (64.0f * M_PI * h9)) * (hMinusr);
      return kernel;
    }

    else{
      return 0.0f;
    }
  }

  /* 
  calculates for pressure force

  - generates the necessary repulsion forces
  - non vanishing gradient near the center
  */

  glm::vec3 SpikyKernel(Particle& i, Particle& j){

    float r = getDistanceParticles(i.position, j.position);

    if(r < 0.0001f || r >= h){
      return glm::vec3(0.0f,0.0f,0.0f);
    }

    glm::vec3 unitVector = (i.position - j.position) / r;

    float h6 = h*h*h*h*h*h;

    float hMinusr = (h - r)*(h - r);

    float scalar = -(45.0f / (M_PI * h6)) * (hMinusr);
    glm::vec3 kernel = scalar * unitVector;

    return kernel;
  }

  /*
  calculates for viscosity

  - viscocity is caused by friction and thus 
  decreases fluids kinetic energy by converting 
  it into heat energy
  - smoothing effect on the velocity field
  */

  float LaplacianKernel(Particle& i, Particle& j){

    float r = getDistanceParticles(i.position, j.position);

    if(r >= h){
      return 0.0f;
    }

    float h6 = h*h*h*h*h*h;

    float kernel = (45.0f / (M_PI * h6)) * (h - r);
    
    return kernel;
  }


  void getDensity(std::vector<std::vector<int>>& neighborCache){
    for(int i = 0; i < particles.size(); i++){
        particles[i].density = 0.0f;
        particles[i].density += particles[i].mass * Poly6Kernel(particles[i], particles[i]);

        for(int j : neighborCache[i]){
            particles[i].density += particles[j].mass * Poly6Kernel(particles[i], particles[j]);
        }
    }
  }

  void getPressure(){
    for(int i = 0; i < particles.size(); i++){
      particles[i].pressure = stiffness * (particles[i].density - restDensity);
    }
  }

  void computeTotalForce(std::vector<std::vector<int>>& neighborCache){

    for(int i = 0; i < particles.size(); i++){
      glm::vec3 totalForce = glm::vec3(0.0f);
      
      for(int j : neighborCache[i]){
        
        Particle& i1 = particles[i];
        Particle& j1 = particles[j];

        glm::vec3 viscosityForce = (viscosity*(j1.mass)) * ((j1.velocity - i1.velocity)/j1.density) * LaplacianKernel(i1, j1);
        glm::vec3 pressureForce = j1.mass * ((i1.pressure + j1.pressure)/(2*j1.density)) * SpikyKernel(i1, j1);

        totalForce += viscosityForce + pressureForce;
      }

      glm::vec3 gravityForce = glm::vec3(0.0f, -0.98f, 0.0f) * particles[i].mass;

      float safeDensity = std::max(particles[i].density, 0.0001f);

      // divided by density because its tiny volumes of fluid (not each particle) --> density is mass per volume
      particles[i].acceleration = (totalForce + gravityForce) / safeDensity;
    }
  }

  void updateFluid(){

    spatialGrid.build(particles);

    // build neighbor cache
    std::vector<std::vector<int>> neighborCache(particles.size());

    for(int i = 0; i < particles.size(); i++){
        neighborCache[i].reserve(64);
        neighborCache[i] = spatialGrid.checkForNeighbors(particles, i);
    }

    getDensity(neighborCache);   
    getPressure();   
    computeTotalForce(neighborCache);
  }

  float getDistanceParticles(glm::vec3 thisPosition, glm::vec3 otherPosition){
    glm::vec3 distanceOfParticles = thisPosition - otherPosition;
    float distance = glm::length(distanceOfParticles);

    return distance;
  }

};