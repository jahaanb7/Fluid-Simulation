/*
fluid_wasm.cpp: the C API the browser calls.

It wraps the same Fluid (SPH) class the desktop app uses. JavaScript only draws: every frame it
calls fs_step() and reads the particle buffer from fs_data().

Build (from the repository root):
  emcmake cmake -S . -B build-web && cmake --build build-web
*/

#include <vector>
#include <glm/glm.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define API extern "C" EMSCRIPTEN_KEEPALIVE
#else
#define API extern "C"
#endif

#include "library/Simulation.h"

namespace {

Fluid fluid;
std::vector<float> buffer;   // x, y, z, speed per particle

// Box half-extents. The 3D box matches main.cpp; the 2D box is wider than it is tall.
float boxW = 2.0f, boxH = 2.0f, boxD = 2.0f;

// Settings per mode. 3D keeps the grid, spacing, h, density, viscosity and gravity of main.cpp;
// the box is a narrower tank and stiffness is higher so the volume holds its shape on screen.
struct Preset { int rows, cols, layers; float spacing, h, targetDensity, stiffness, viscosity, gravity, w, hgt, d; glm::vec3 seed; };

const Preset preset3D = { 15, 15, 15, 0.08f, 0.20f, 250.0f, 8.0f, 0.08f, 2.0f, 1.1f, 1.6f, 0.7f, {-0.45f, 0.7f, 0.0f} };
const Preset preset2D = { 34, 26, 1,  0.07f, 0.20f, 120.0f, 6.0f, 0.10f, 4.0f, 2.4f, 1.5f, 0.0f, {-1.2f, -0.2f, 0.0f} };

void fillBuffer(){
  buffer.resize(fluid.particles.size() * 4);
  for(size_t i = 0; i < fluid.particles.size(); i++){
    const Particle& p = fluid.particles[i];
    buffer[i*4 + 0] = p.position.x;
    buffer[i*4 + 1] = p.position.y;
    buffer[i*4 + 2] = p.position.z;
    buffer[i*4 + 3] = glm::length(p.velocity);
  }
}

}

// dims: 2 or 3. Resets the parameters to that mode's preset and reseeds the block.
API void fs_reset(int dims){
  const Preset& s = (dims == 2) ? preset2D : preset3D;
  fluid.setDimensions(dims);
  fluid.setSmoothingRadius(s.h);
  fluid.targetDensity = s.targetDensity;
  fluid.stiffness = s.stiffness;
  fluid.viscosity = s.viscosity;
  fluid.gravity = s.gravity;
  boxW = s.w; boxH = s.hgt; boxD = (dims == 2) ? 1.0f : s.d;

  seedBlock(fluid, s.seed, s.rows, s.cols, s.layers, s.spacing, s.spacing * 0.5f);
  fluid.updateFluid();
  fillBuffer();
}

// Advance by dt seconds in `substeps` equal steps (each capped at 16 ms like main.cpp).
API void fs_step(float dt, int substeps){
  if(substeps < 1) substeps = 1;
  float sub = dt / substeps;
  if(sub > 0.016f) sub = 0.016f;
  for(int i = 0; i < substeps; i++){
    stepFluid(fluid, sub, boxW, boxH, boxD);
  }
  fillBuffer();
}

API int fs_count(){ return (int)fluid.particles.size(); }
API float* fs_data(){ return buffer.data(); }
API float fs_box(int axis){ return axis == 0 ? boxW : axis == 1 ? boxH : boxD; }

// 0 viscosity, 1 stiffness, 2 gravity, 3 smoothing radius h, 4 target density
API void fs_set(int key, float v){
  switch(key){
    case 0: fluid.viscosity = v; break;
    case 1: fluid.stiffness = v; break;
    case 2: fluid.gravity = v; break;
    case 3: fluid.setSmoothingRadius(v); break;
    case 4: fluid.targetDensity = v; break;
  }
}

API float fs_get(int key){
  switch(key){
    case 0: return fluid.viscosity;
    case 1: return fluid.stiffness;
    case 2: return fluid.gravity;
    case 3: return fluid.h;
    case 4: return fluid.targetDensity;
    case 5: return fluid.particles.empty() ? 0.0f : fluid.particles[0].mass;
  }
  return 0.0f;
}

API void fs_impulse(float x, float y, float z, float radius, float strength){
  radialImpulse(fluid, {x, y, z}, radius, strength);
}

API void fs_stir(float x, float y, float z, float vx, float vy, float vz, float radius, float strength){
  stir(fluid, {x, y, z}, {vx, vy, vz}, radius, strength);
}

// Density of particle i as the simulation computed it on the last step.
API float fs_density(int i){
  if(i < 0 || i >= (int)fluid.particles.size()) return 0.0f;
  return fluid.particles[i].density;
}
