#include <GLFW/glfw3.h>
#include "Particle.h"

std::vector<Particle> particles;

void drawParticleGrid(int rows, int columns, float spacing, Particle& particle){
  
  particles.clear();
  particles.reserve(rows*columns);

  for(int i = 0; i < rows; i++){
    for(int j = 0; j < columns; j++){
      
      glm::vec2 gridPosition = {
        particle.position.x + (j * spacing),  // x moves with column
        particle.position.y + (i * spacing)   // y moves with row
      };
      
      particles.emplace_back(gridPosition, particle.velocity, particle.radius);
    }
  }
}

void update(float deltaTime, int WIDTH, int HEIGHT){
    for(auto& particle : particles){
      particle.updatePosition(deltaTime);
      particle.boundaryCollision(WIDTH, HEIGHT);
      particle.drawParticle();
  }
}