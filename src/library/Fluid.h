#include <iostream>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <omp.h>


#include "Particle.h"
#include "SpatialGrid.h"

class Fluid{
  
  public:

    std::vector<Particle> particles;
    std::vector<std::vector<int>> neighborCache;

    const float h = 0.2f;   
    const float targetDensity = 20000.0f; 
    const float stiffness = 2.0f;    
    const float viscosity = 0.05f;   
    const float gravity = 200.0f;  
    const float pressureForce = 1.0f;

    // constants for smoothing kernel funcitions:
    const float h2 = h*h;
    const float h6 = h*h*h*h*h*h;
    const float h9 = h*h*h*h*h*h*h*h*h;

    const float poly6Const = 315.0f / (64.0f * M_PI * h9);
    const float spikyConst = 45.0f  / (M_PI * h6);
    const float lapConst = 45.0f  / (M_PI * h6);

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

  void initParticles(){
    neighborCache.resize(particles.size());

    std::cout << "Particles:    " << particles.size() << std::endl;
  }

  // calculates for density force
  float Poly6Kernel(float r){

    if(r >= h){
      return 0.0f;
    }

    float diff = h2 - r*r;

    return poly6Const * (diff*diff*diff);
  }

  /* 
  calculates for pressure force

  - generates the necessary repulsion forces
  - non vanishing gradient near the center
  */

  glm::vec3 SpikyKernel(float r, glm::vec3 dir){
    if(r < 0.0001f || r >= h){
      return glm::vec3(0.0f);
    }

    float hr = h - r;

    return (-spikyConst * hr * hr) * dir;
  }

  /*
  calculates for viscosity

  - viscocity is caused by friction and thus 
  decreases fluids kinetic energy by converting 
  it into heat energy
  - smoothing effect on the velocity field
  */

  float LaplacianKernel(float r){

    if(r >= h) return 0.0f;

    return lapConst * (h - r);
  }

  void getDensity(){
    #pragma omp parallel for schedule(dynamic)

    for(int i = 0; i < particles.size(); i++){
        particles[i].density = particles[i].mass * poly6Const * (h2*h2*h2);

        for(int j : neighborCache[i]){
          glm::vec3 difference = particles[i].position - particles[j].position;

          float r2 = glm::dot(difference, difference);

          if(r2 >= h2) continue;

          float r = std::sqrt(r2);

          particles[i].density += particles[j].mass * Poly6Kernel(r);
        }
    }
  }

  void getPressure(){
    #pragma omp parallel for

    for(int i = 0; i < particles.size(); i++){
      float pressure = stiffness * (particles[i].density - targetDensity);
      
      particles[i].pressure = std::max(-25.0f, pressure) * pressureForce;
    }
  }

  void computeTotalForce(){
    #pragma omp parallel for

    for(int i = 0; i < particles.size(); i++){
      glm::vec3 pressureForce  = glm::vec3(0.0f);
      glm::vec3 viscosityForce = glm::vec3(0.0f);

      
      for(int j : neighborCache[i]){
        
        Particle& i1 = particles[i];
        Particle& j1 = particles[j];

        glm::vec3 difference = i1.position - j1.position;
        float r2 = glm::dot(difference, difference);

        if(r2 >= h2) continue;

        float r = std::sqrt(r2);
        glm::vec3 dir = difference/r;

        // pressure force - symmetric formulation
        float avgPressure = (i1.pressure + j1.pressure) / (2.0f * j1.density);
        pressureForce += j1.mass * avgPressure * SpikyKernel(r, dir);

        // viscosity force
        float lap = LaplacianKernel(r);
        viscosityForce += viscosity * j1.mass * ((j1.velocity - i1.velocity) / j1.density) * lap;
      }

      glm::vec3 gravityForce = glm::vec3(0.0f, -gravity, 0.0f) * particles[i].mass;

      float safeDensity = std::max(particles[i].density, 1e-6f);

      // divided by density because its tiny volumes of fluid (not each particle) --> density is mass per volume
      particles[i].acceleration = (pressureForce + viscosityForce + gravityForce) / safeDensity;
    }
  }

  void updateFluid(){
    spatialGrid.build(particles);

    #pragma omp parallel for schedule(dynamic)

    for(int i = 0; i < particles.size(); i++){
        neighborCache[i] = spatialGrid.checkForNeighbors(particles, i);
    }

    getDensity();   
    getPressure();   
    computeTotalForce();
  }

  float getDistanceParticles(glm::vec3 thisPosition, glm::vec3 otherPosition){
    glm::vec3 distanceOfParticles = thisPosition - otherPosition;
    float distance = glm::length(distanceOfParticles);

    return distance;
  }
};