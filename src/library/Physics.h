#include <GLFW/glfw3.h>
#include "Particle.h"

std::vector<Particle> particles;

void VerletIntegration(Particle& particle, float deltaTime){
    
    glm::vec3 oldAcceleration = particle.acceleration;
    particle.position += particle.velocity * deltaTime + 0.5f * oldAcceleration * deltaTime * deltaTime;

    particle.acceleration = glm::vec3(0.0f, -98.0f, 0.0f);
    particle.velocity += 0.5f * (oldAcceleration + particle.acceleration) * deltaTime;
}

void drawParticleGrid3D(int rows, int columns, int zRange, float spacing, Particle& particle){
  particles.clear();
  particles.reserve(rows*columns*zRange);

  for(int z = 0; z < zRange; z++){
    for(int i = 0; i < rows; i++){
      for(int j = 0; j < columns; j++){

        glm::vec3 gridPosition = {
          particle.position.x + (j * spacing), // x moves with column
          particle.position.y + (i * spacing), // y moves with row
          particle.position.z + (z * spacing)  // z moves with zRange
        };

        particles.emplace_back(gridPosition, particle.velocity, particle.radius);
      }
    }
  }
}

void update(float deltaTime, int WIDTH, int HEIGHT, int DEPTH){
  for(auto& particle : particles){
    VerletIntegration(particle, deltaTime);
  }

  float damping = 0.95f;

  for(int i = 0; i < particles.size(); i++){
    for(int j = i + 1; j < particles.size(); j++){

      glm::vec3 distanceOfParticles = particles[j].position - particles[i].position;
      float distance = glm::length(distanceOfParticles);
      float sumOfRadius = particles[i].radius + particles[j].radius;

      if(distance < sumOfRadius){
        glm::vec3 temp = particles[i].velocity;
        particles[i].velocity = particles[j].velocity * damping;
        particles[j].velocity = temp * damping;

        glm::vec3 normal = glm::normalize(distanceOfParticles);
        float overlap = sumOfRadius - distance;

        particles[i].position -= normal * (overlap * 0.5f);
        particles[j].position += normal * (overlap * 0.5f);
      }
    }
  }

  for(auto& particle : particles){
    particle.boundaryCollision(WIDTH/2.0f, HEIGHT/2.0f, DEPTH/2.0f);
    particle.drawParticle3D(5, 5);
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
