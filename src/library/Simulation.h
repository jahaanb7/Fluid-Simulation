#pragma once

/*
Simulation.h: the parts of a time step that do not draw anything.

The desktop app (Physics.h) and the WebAssembly build (web/fluid_wasm.cpp) both call these, so the
browser runs exactly the same SPH as the native program. Nothing here touches OpenGL.
*/

#include <cmath>
#include <glm/glm.hpp>

#include "Fluid.h"

// Fill the fluid with a block of particles centred on `center`.
// For the 2D mode pass layers = 1: the block is flat and z stays at 0.
inline void seedBlock(Fluid& fluid, glm::vec3 center, int rows, int columns, int layers, float spacing, float radius){
  fluid.particles.clear();
  fluid.particles.reserve(rows * columns * layers);

  float offsetX = (columns - 1) * spacing / 2.0f;
  float offsetY = (rows - 1) * spacing / 2.0f;
  float offsetZ = (layers - 1) * spacing / 2.0f;

  for(int z = 0; z < layers; z++){
    for(int i = 0; i < rows; i++){
      for(int j = 0; j < columns; j++){
        glm::vec3 p = {
          center.x + j * spacing - offsetX,
          center.y + i * spacing - offsetY,
          (layers == 1) ? 0.0f : center.z + z * spacing - offsetZ
        };
        fluid.particles.emplace_back(p, glm::vec3(0.0f), radius);
      }
    }
  }

  fluid.neighborCache.assign(fluid.particles.size(), {});
}

// One step: integrate with the forces from the previous step, keep particles in the box,
// then recompute density, pressure and forces (same order as the original update()).
inline void stepFluid(Fluid& fluid, float deltaTime, float WIDTH, float HEIGHT, float DEPTH){
  const bool planar = fluid.dimensions == 2;

  for(auto& particle : fluid.particles){
    if(particle.isBoundary){
      continue;
    }

    particle.velocity += particle.acceleration * deltaTime;
    particle.position += particle.velocity * deltaTime;

    if(planar){
      particle.position.z = 0.0f;
      particle.velocity.z = 0.0f;
      particle.boundaryCollision(WIDTH, HEIGHT, 1.0f);
    } else {
      particle.boundaryCollision(WIDTH, HEIGHT, DEPTH);
    }
  }

  fluid.updateFluid();
}

// Push particles away from a point (the original "Inject Pulse").
inline void radialImpulse(Fluid& fluid, const glm::vec3& point, float radius, float strength){
  for(auto& p : fluid.particles){
    if(p.isBoundary) continue;

    glm::vec3 diff = p.position - point;
    if(fluid.dimensions == 2) diff.z = 0.0f;

    float dist = glm::length(diff);
    if(dist <= 0.00001f || dist > radius) continue;

    float t = 1.0f - (dist / radius); // 0..1 falloff
    glm::vec3 dir = diff / dist;      // outward

    p.velocity += dir * (strength * t);
    p.velocity += glm::vec3(0.0f, strength * 0.25f * t, 0.0f);
  }
}

// Drag the fluid along with the pointer: particles near `point` are pulled toward `velocity`.
inline void stir(Fluid& fluid, const glm::vec3& point, const glm::vec3& velocity, float radius, float strength){
  for(auto& p : fluid.particles){
    if(p.isBoundary) continue;

    glm::vec3 diff = p.position - point;
    if(fluid.dimensions == 2) diff.z = 0.0f;

    float dist = glm::length(diff);
    if(dist > radius) continue;

    float t = 1.0f - (dist / radius);
    glm::vec3 dv = (velocity - p.velocity) * (strength * t * t);
    if(fluid.dimensions == 2) dv.z = 0.0f;
    p.velocity += dv;
  }
}
