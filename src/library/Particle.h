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
    float pressure;
    float mass;
    float radius;

    // boundary particles to prevent highly dense particles near the boundary
    bool isBoundary = false; 

    Particle(glm::vec3 pos, glm::vec3 vel, float r){
      position = pos;
      velocity = vel;
      radius = r;

      // set values for fluid properties
      mass = 0.76f;
      density = 0.0f;
      pressure = 0.0f;
      acceleration = glm::vec3(0.0f);
    }

  void updatePosition(float deltaTime){
    position += velocity * deltaTime + 0.5f * acceleration * deltaTime * deltaTime;
    velocity += acceleration * deltaTime;
  }

  void boundaryCollision(float WIDTH, float HEIGHT, float DEPTH){
    const float damping  = -0.3f;

    if(position.x + radius > WIDTH){
        position.x = WIDTH - radius;
        velocity.x *= damping;
    }

    if(position.x - radius < -WIDTH){
        position.x = -WIDTH + radius;
        velocity.x *= damping;
    }

    if(position.y + radius > HEIGHT){
        position.y = HEIGHT - radius;
        velocity.y *= damping;
    }

    if(position.y - radius < -HEIGHT){
        position.y = -HEIGHT + radius;
        velocity.y *= damping;
    }

    if (position.z + radius > DEPTH) {
      position.z = DEPTH - radius;
      velocity.z *= damping;
    }
    if(position.z - radius < -DEPTH) {
      position.z = -DEPTH + radius;
      velocity.z *= damping;
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

  void drawParticle3D(int lats, int longs, glm::vec3 color){

    GLfloat diff[] = {color.r, color.g, color.b, 1.0f};
    GLfloat spec[] = {0.4f, 0.4f, 0.4f, 1.0f};

    glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT, GL_SHININESS, 32.0f);

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