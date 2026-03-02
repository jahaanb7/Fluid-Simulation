#include "Particle.h"

void drawParticleGrid(int rows, int columns, float spacing, Particle& particle){
  
  std::vector<Particle> particles;

  particles.reserve(rows*columns);

  for(int i = 0; i < rows; i++){
    for(int j = 0; j < columns; j++){
      
      glm::vec2 gridPosition = {
        particle.position.x + j * spacing,  // x moves with column
        particle.position.y + i * spacing   // y moves with row
      };
      
      particles.emplace_back(gridPosition, particle.velocity, particle.radius);
    }
  }

  for(auto& particle : particles){
    particle.drawParticle();
  }
}