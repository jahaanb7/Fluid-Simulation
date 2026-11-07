#include <GLFW/glfw3.h>
#include "Particle.h"
#include <glm/glm.hpp>
#include "Fluid.h"
#include "Simulation.h"

Fluid fluid;

void VerletIntegration(Particle& particle, float deltaTime){
    
    glm::vec3 oldAcceleration = particle.acceleration;
    particle.position += particle.velocity * deltaTime + 0.5f * oldAcceleration * deltaTime * deltaTime;
    particle.velocity += 0.5f * (oldAcceleration + particle.acceleration) * deltaTime;
}


void drawParticleGrid3D(int rows, int columns, int zRange, float spacing, Particle& particle){
  seedBlock(fluid, particle.position, rows, columns, zRange, spacing, particle.radius);
}

// 2D mode: one flat layer at z = 0
void drawParticleGrid2D(int rows, int columns, float spacing, Particle& particle){
  seedBlock(fluid, particle.position, rows, columns, 1, spacing, particle.radius);
}

glm::vec3 velocityColor(float speed, float maxSpeed = 3.0f){

  float s = std::min(speed / maxSpeed, 1.0f);

  // low --> navy blue, high --> sky blue
  glm::vec3 low(0.00f, 0.20f, 0.87f);   
  glm::vec3 high(0.53, 0.61, 0.92);

  return glm::mix(low, high, s);
}

void circleCollision(){
  float damping = 0.95f;

  #pragma omp parallel for schedule(dynamic)

  for(int i = 0; i < fluid.particles.size(); i++){
    for(int j = i + 1; j < fluid.particles.size(); j++){

      glm::vec3 distanceOfParticles = fluid.particles[j].position - fluid.particles[i].position;
      float distance = glm::length(distanceOfParticles);
      float sumOfRadius = fluid.particles[i].radius + fluid.particles[j].radius;

      if(distance < sumOfRadius){
        glm::vec3 temp = fluid.particles[i].velocity;
        fluid.particles[i].velocity = fluid.particles[j].velocity * damping;
        fluid.particles[j].velocity = temp * damping;

        glm::vec3 normal = glm::normalize(distanceOfParticles);
        float overlap = sumOfRadius - distance;

        fluid.particles[i].position -= normal * (overlap * 0.5f);
        fluid.particles[j].position += normal * (overlap * 0.5f);
      }
    }
  }
}

void update(float deltaTime, float WIDTH, float HEIGHT, float DEPTH){

  // integration, boundary and SPH forces live in Simulation.h (shared with the web build)
  stepFluid(fluid, deltaTime, WIDTH, HEIGHT, DEPTH);

  glEnable(GL_LIGHTING);

  for (auto& particle : fluid.particles) {
    glm::vec3 col = velocityColor(glm::length(particle.velocity));
    particle.drawParticle3D(7, 7, col);
  }
}

// 3D Boundary Box - collision and containment
void drawBoundaryBox(float width, float height, float depth) {
    float x = width;
    float y = height;
    float z = depth;

    glDisable(GL_LIGHTING);
    glColor3f(1.0f,1.0f,1.0f); 

    glBegin(GL_LINES);

    glVertex3f(-x, -y, -z); glVertex3f( x, -y, -z);
    glVertex3f( x, -y, -z); glVertex3f( x,  y, -z);
    glVertex3f( x,  y, -z); glVertex3f(-x,  y, -z);
    glVertex3f(-x,  y, -z); glVertex3f(-x, -y, -z);

    glVertex3f(-x, -y,  z); glVertex3f( x, -y,  z);
    glVertex3f( x, -y,  z); glVertex3f( x,  y,  z);
    glVertex3f( x,  y,  z); glVertex3f(-x,  y,  z);
    glVertex3f(-x,  y,  z); glVertex3f(-x, -y,  z);

    glVertex3f(-x, -y, -z); glVertex3f(-x, -y,  z);
    glVertex3f( x, -y, -z); glVertex3f( x, -y,  z);
    glVertex3f( x,  y, -z); glVertex3f( x,  y,  z);
    glVertex3f(-x,  y, -z); glVertex3f(-x,  y,  z);

    glEnd();

    glEnable(GL_LIGHTING);
}
