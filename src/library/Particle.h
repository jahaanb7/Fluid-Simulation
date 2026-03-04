#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

#pragma once

class Particle{
  public:
    glm::vec2 position;
    glm::vec2 velocity;

    glm::vec2 acceleration;

    float density;
    float mass;

    // variables for rendering
    int radius;
    int numSegments = 20;

    Particle(glm::vec2 pos, glm::vec2 vel, int r){
      position = pos;
      velocity = vel;
      radius = r;

      acceleration = glm::vec2(0.0f, -98.0f);
    }

  void updatePosition(float deltaTime){
    position += velocity * deltaTime + 0.5f * acceleration * deltaTime * deltaTime;
    velocity += acceleration * deltaTime;
  }

  void boundaryCollision(int screenWidth, int screenHeight){
    if(position.x + radius > screenWidth){
        position.x = screenWidth - radius;
        velocity.x *= -1;
    }

    if(position.x - radius < -screenWidth){
        position.x = -screenWidth + radius;
        velocity.x *= -1;
    }

    if(position.y + radius > screenHeight){
        position.y = screenHeight - radius;
        velocity.y *= -1;
    }

    if(position.y - radius < -screenHeight){
        position.y = -screenHeight + radius;
        velocity.y *= -1;
    }
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