#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <cmath>

class Particle{
  public:
    glm::vec2 position;
    glm::vec2 velocity;

    Particle(glm::vec2 pos, glm::vec2 vel){
      position = pos;
      velocity = vel;
    }

  void drawParticle(float radius, int numSegments){

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f,1.0f,1.0f); 

    glVertex2f(position.x, position.y);

    for(int i = 0; i <= numSegments; i++){

      float theta = 2.0f * M_PI * float(i) / float(numSegments);

      float x = radius * cos(theta);
      float y = radius * sin(theta);

      glVertex2f(x + position.x, y + position.y);
    }
    glEnd();
  }
};