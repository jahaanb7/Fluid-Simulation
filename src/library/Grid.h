#include <vector>
#include <cmath>
#include <glm/glm.hpp>

class SpatialGrid {
public:
  float cellSize;

  int numCellsX;
  int numCellsY;
  int numCellsZ;

  std::vector<std::vector<int>> cells;

  SpatialGrid(){
    
  }
};
