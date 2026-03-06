#include <GLFW/glfw3.h>
#include "Particle.h"
#include <glm/glm.hpp>
#include "Fluid.h"

Fluid fluid;

void VerletIntegration(Particle& particle, float deltaTime){
    
    glm::vec3 oldAcceleration = particle.acceleration;
    particle.position += particle.velocity * deltaTime + 0.5f * oldAcceleration * deltaTime * deltaTime;
    particle.velocity += 0.5f * (oldAcceleration + particle.acceleration) * deltaTime;
    particle.velocity *= 0.998f;
}

void drawParticleGrid3D(int rows, int columns, int zRange, float spacing, Particle& particle){
  fluid.particles.clear();
  fluid.particles.reserve(rows*columns*zRange);

  float offsetX = (columns - 1) * spacing/2.0f;
  float offsetY = (rows - 1) * spacing/2.0f;
  float offsetZ = (zRange - 1) * spacing/2.0f;

  for(int z = 0; z < zRange; z++){
    for(int i = 0; i < rows; i++){
      for(int j = 0; j < columns; j++){

        glm::vec3 gridPosition = {
          particle.position.x + (j * spacing) - offsetX, // x moves with column
          particle.position.y + (i * spacing) - offsetY, // y moves with row
          particle.position.z + (z * spacing) - offsetZ  // z moves with zRange
        };

        fluid.particles.emplace_back(gridPosition, particle.velocity, particle.radius);
      }
    }
  }
}

void update(float deltaTime, int WIDTH, int HEIGHT, int DEPTH){

  fluid.updateFluid();

  for(auto& particle : fluid.particles){
    VerletIntegration(particle, deltaTime);
    particle.boundaryCollision(WIDTH/2.0f, HEIGHT/2.0f, DEPTH/2.0f);
  }

    glDisable(GL_LIGHTING);
    glPointSize(5.0f);

    glBegin(GL_POINTS);
    for(auto& particle : fluid.particles){

        // get speed (magnitude of velocity)
        float speed = glm::length(particle.velocity);

        // clamp speed to a max value for color mapping
        float maxSpeed = 8.0f;
        float t = std::min(speed / maxSpeed, 1.0f);

        // interpolate between two colors based on t
        // slow = deep blue (0.0, 0.2, 0.8)
        // fast = bright cyan/white (0.0, 1.0, 1.0)
        glm::vec3 slowColor = glm::vec3(0.0f, 0.2f, 0.8f);
        glm::vec3 fastColor = glm::vec3(1.0f, 0.8f, 0.2f);
        glm::vec3 color = slowColor + t * (fastColor - slowColor);

        glColor3f(color.r, color.g, color.b);
        glVertex3f(particle.position.x, particle.position.y, particle.position.z);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

void circleCollision(){
  float damping = 0.95f;

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

// 2D Boundary Box - collision and containment
void drawBoundaryBox(float width, float height, float depth) {
    float x = width / 2.0f;
    float y = height / 2.0f;
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
