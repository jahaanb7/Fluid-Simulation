#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <unordered_map>

#include "Particle.h"

#pragma once

class SpatialGrid {

  std::unordered_map<int, std::vector<int>> grid;
  float cellSize;

  public:
    SpatialGrid(float smoothingRadius){
      cellSize = smoothingRadius;
    }

    glm::ivec3 getCellCoord(const glm::vec3& position) {
      return glm::ivec3(
        (int)std::floor(position.x/cellSize),
        (int)std::floor(position.y/cellSize),
        (int)std::floor(position.z/cellSize)
      );
    }

    // Hash the 3D coords to a single int value (prime numbers help to reduce redundacy of cells)
    int hashCell(const glm::ivec3& cell) {
      return (cell.x * 73856093)^(cell.y * 19349663)^(cell.z * 83492791);
    }

    void build(std::vector<Particle>& particles) {

      grid.clear();

      for (int i = 0; i < (int)particles.size(); i++){

        glm::ivec3 cell = getCellCoord(particles[i].position);
        int key = hashCell(cell);
        grid[key].push_back(i);  // store the index of particle
      }
    }

    std::vector<int> checkForNeighbors(std::vector<Particle>& particles, int index) {

      std::vector<int> neighbors;

      glm::ivec3 centerCell = getCellCoord(particles[index].position);

      // iterate over 3x3x3 = 27 surrounding cells
      for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
          for (int z = -1; z <= 1; z++) {

            glm::ivec3 neighborCell = centerCell + glm::ivec3(x, y, z);
            int key = hashCell(neighborCell);

            // check if any particles live in this cell
            auto it = grid.find(key);
            if (it == grid.end()){
              continue;
            }

            // add each particle in that cell to neighbors
            for (int j : it->second) {
              if (j == index){
                continue;
              }
              
              neighbors.push_back(j);
            }
          }
        }
      }

      return neighbors;
    }
};
