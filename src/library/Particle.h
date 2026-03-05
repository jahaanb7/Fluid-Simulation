#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

#pragma once

class Particle{
  public:

    //particle variables
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    // properties of particles
    float density;
    int radius;

    Particle(glm::vec3 pos, glm::vec3 vel, int r){
      position = pos;
      velocity = vel;
      radius = r;

      acceleration = glm::vec3(0.0f, -98.0f, 0.0f);
    }

  void updatePosition(float deltaTime){
    position += velocity * deltaTime + 0.5f * acceleration * deltaTime * deltaTime;
    velocity += acceleration * deltaTime;
  }

  void boundaryCollision(int screenWidth, int screenHeight, int screenDepth){
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

    if (position.z + radius > screenDepth) {
      position.z = screenDepth - radius;
      velocity.z *= -1;
    }
    if(position.z - radius < -screenDepth) {
      position.z = -screenDepth + radius;
      velocity.z *= -1;
    }
  }

  void drawParticle(int numSegments){

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

  void drawParticle3D(int lats, int longs){
    for(int i = 0; i < lats; i++){
      float lat0 = M_PI * (-0.5f + (float)i / lats);
      float z0  = sin(lat0) * radius;
      float zr0 = cos(lat0) * radius;

      float lat1 = M_PI * (-0.5f + (float)(i+1) / lats);
      float z1 = sin(lat1) * radius;
      float zr1 = cos(lat1) * radius;

      glBegin(GL_TRIANGLE_STRIP);
      for(int j = 0; j <= longs; j++){

        float lng = 2 * M_PI * (float)(j) / longs;
        float x = cos(lng);
        float y = sin(lng);

        glm::vec3 v1 = position + glm::vec3(x*zr0, y*zr0, z0);
        glm::vec3 n1 = glm::normalize(glm::vec3(x*zr0, y*zr0, z0));

        glNormal3f(n1.x, n1.y, n1.z);
        glVertex3f(v1.x, v1.y, v1.z);

        glm::vec3 v2 = position + glm::vec3(x*zr1, y*zr1, z1);
        glm::vec3 n2 = glm::normalize(glm::vec3(x*zr1, y*zr1, z1));

        glNormal3f(n2.x, n2.y, n2.z);
        glVertex3f(v2.x, v2.y, v2.z);
      }
      glEnd();
    }
  }
};