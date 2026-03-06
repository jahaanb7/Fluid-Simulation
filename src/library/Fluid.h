#include <iostream>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

#include "Particle.h"

class Fluid{
  
  public:

    std::vector<Particle> particles;

    // Properties of fluids (restDensity is the density of the fluid we want to eventually achieve)
    const float viscosity = 0.1f;
    const float restDensity = 0.0245233f;
    const float stiffness = 90.0f;

    //smoothing radius for kernel
    const float smoothingRadius = 80.0f;

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

    float distanceParticles = getDistanceParticles(i.position, j.position);

    if(distanceParticles < smoothingRadius){
      float kernel = (315.0f / (64.0f * M_PI * (pow(smoothingRadius, 9)))) * (pow(((smoothingRadius*smoothingRadius) - (distanceParticles*distanceParticles)), 3));
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

    float distanceParticles = getDistanceParticles(i.position, j.position);

    if(distanceParticles < 0.0001f || distanceParticles >= smoothingRadius){
      return glm::vec3(0.0f,0.0f,0.0f);
    }

    glm::vec3 unitVector = (i.position - j.position) / distanceParticles;

    float scalar = -(45.0f / (M_PI * pow(smoothingRadius, 6.0f))) * (pow((smoothingRadius - distanceParticles), 2.0f));
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

    float distanceParticles = getDistanceParticles(i.position, j.position);

    if(distanceParticles >= smoothingRadius){
      return 0.0f;
    }

    float kernel = (45.0f / (M_PI * pow(smoothingRadius, 6))) * (smoothingRadius - distanceParticles);
    
    return kernel;
  }


  void getDensity(){
    for(int i = 0; i < particles.size(); i++){
        particles[i].density = 0.0f;
        for(int j = 0; j < particles.size(); j++){
            particles[i].density += particles[j].mass * Poly6Kernel(particles[i], particles[j]);
        }
    }
  }

  void getPressure(){
    for(int i = 0; i < particles.size(); i++){
      particles[i].pressure = stiffness * (particles[i].density - restDensity);
    }
  }

  void computeTotalForce(){

    for(int i = 0; i < particles.size(); i++){
      glm::vec3 totalForce = glm::vec3(0.0f);

      for(int j = 0; j < particles.size(); j++){

        if(i == j) continue;
        
        Particle& i1 = particles[i];
        Particle& j1 = particles[j];

        glm::vec3 viscosityForce = (viscosity*(j1.mass)) * ((j1.velocity - i1.velocity)/j1.density) * LaplacianKernel(i1, j1);
        glm::vec3 pressureForce = j1.mass * ((i1.pressure + j1.pressure)/(2*j1.density)) * SpikyKernel(i1, j1);

        totalForce += viscosityForce + pressureForce;
      }

      glm::vec3 gravityForce = glm::vec3(0.0f, -0.98f, 0.0f) * particles[i].mass;

      // divided by density because its tiny volumes of fluid (not each particle) --> density is mass per volume
      particles[i].acceleration = (totalForce + gravityForce) / particles[i].density;
    }
  }

  void updateFluid(){
    getDensity();   
    
    std::cout << "density[0]: " << particles[0].density << std::endl;     

    getPressure();   
    
    std::cout << "pressure[0]: " << particles[0].pressure << std::endl;

    computeTotalForce();
  }

  float getDistanceParticles(glm::vec3 thisPosition, glm::vec3 otherPosition){
    glm::vec3 distanceOfParticles = thisPosition - otherPosition;
    float distance = glm::length(distanceOfParticles);

    return distance;
  }

};