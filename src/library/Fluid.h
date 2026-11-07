#pragma once

#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#ifdef _OPENMP
#include <omp.h>
#endif


#include "Particle.h"
#include "SpatialGrid.h"

class Fluid{
  
  public:

    std::vector<Particle> particles;
    std::vector<std::vector<int>> neighborCache;

    // 3 = the original 3D simulation, 2 = a planar version (z is locked to 0)
    int dimensions = 3;

    float h = 0.2f;
    float targetDensity = 250.0f;
    float stiffness = 0.7f;
    float viscosity = 0.08f;
    float gravity = 2.0f;

    // constants for smoothing kernel funcitions (recomputed when h or the dimension changes):
    float h2, h6, h9;
    float poly6Const, spikyConst, lapConst;

    SpatialGrid spatialGrid{h};

    Fluid(){
      setKernelConstants();
    }

    /*
    The kernels must integrate to 1 over their support, so the constants depend on the dimension:

                    3D                    2D
    Poly6     315 / (64 pi h^9)       4 / (pi h^8)
    Spiky     45 / (pi h^6)           30 / (pi h^5)    (gradient)
    Laplacian 45 / (pi h^6)           40 / (pi h^5)    (viscosity)
    */
    void setKernelConstants(){
      h2 = h*h;
      h6 = h2*h2*h2;
      h9 = h6*h2*h;

      if(dimensions == 2){
        poly6Const = 4.0f  / (float(M_PI) * h6 * h2);
        spikyConst = 30.0f / (float(M_PI) * h2 * h2 * h);
        lapConst   = 40.0f / (float(M_PI) * h2 * h2 * h);
      } else {
        poly6Const = 315.0f / (64.0f * float(M_PI) * h9);
        spikyConst = 45.0f  / (float(M_PI) * h6);
        lapConst   = 45.0f  / (float(M_PI) * h6);
      }
    }

    void setSmoothingRadius(float newH){
      h = newH;
      setKernelConstants();
      spatialGrid = SpatialGrid(h);
    }

    void setDimensions(int dims){
      dimensions = (dims == 2) ? 2 : 3;
      setKernelConstants();
    }

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

  glm::vec3 SpikyKernelGradient(float r, glm::vec3 dir){
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

      particles[i].pressure = std::max(0.0f, pressure);
    }
  }

  void computeTotalForce(){
    #pragma omp parallel for

    for(int i = 0; i < particles.size(); i++){
      glm::vec3 pressureForce  = glm::vec3(0.0f);
      glm::vec3 viscosityForce = glm::vec3(0.0f);

      Particle& i1 = particles[i];
      float safeDensity_i = std::max(i1.density, 1e-6f);

      for(int j : neighborCache[i]){
        
        const Particle j1 = particles[j]; 

        glm::vec3 difference = i1.position - j1.position;
        float r2 = glm::dot(difference, difference);

        if(r2 >= h2 || r2 < 0.0001f * 0.0001f){
          continue;
        }

        float r = std::sqrt(r2);
        glm::vec3 dir = difference/r;

        float safeDensity_j = std::max(j1.density, 1e-6f);

        // pressure force - symmetric formulation
        float pressure = (i1.pressure / (safeDensity_i * safeDensity_i)) + (j1.pressure / (safeDensity_j * safeDensity_j));
        pressureForce -= j1.mass * pressure * SpikyKernelGradient(r, dir);

        // viscosity force
        float lap = LaplacianKernel(r);
        viscosityForce += viscosity * (j1.mass / safeDensity_j) * (j1.velocity - i1.velocity) * lap;      
      }
      
      i1.acceleration  = pressureForce + viscosityForce;
      i1.acceleration += glm::vec3(0.0f, -gravity, 0.0f) * i1.mass;

      if(dimensions == 2){
        i1.acceleration.z = 0.0f;
      }
    }
  }

  void updateFluid(){
    spatialGrid.build(particles);

    if(neighborCache.size() != particles.size()){
      neighborCache.resize(particles.size());
    }

    #pragma omp parallel for schedule(dynamic)

    for(int i = 0; i < (int)particles.size(); i++){
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
