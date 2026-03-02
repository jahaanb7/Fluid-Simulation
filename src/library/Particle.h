#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

#pragma once

class Particle{
  public:
    glm::vec2 position;
    glm::vec2 velocity;

    // variables for rendering
    int radius;
    int numSegments = 20;

    Particle(glm::vec2 pos, glm::vec2 vel, int r){
      position = pos;
      velocity = vel;
      radius = r;
    }

  void drawParticle(){

    glm::vec2 center = glm::vec2(position.x, position.y);

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f,1.0f,1.0f); 

    glVertex2f(center.x, center.y);

    for(int i = 0; i <= numSegments; i++){

      float theta = 2.0f * M_PI * float(i) / float(numSegments);

      float x = radius * cos(theta);
      float y = radius * sin(theta);

      glVertex2f(x + position.x, y + position.y);
    }
    glEnd();
  }
};