### Fluid Simulation

A C++/OpenGL fluid simulation using GLFW + ImGui (heavily inspired by Sebastian Lague).

## Smoothed Particle Hydrodynamics
Imagine water made of a lot of tiny balls. In SPH, each ball is a “particle”, and we pretend the water is made of these particles.

The hard part is that water is continuous (it’s smooth), but particles are discrete (they are separate dots). So SPH uses a trick: it “smears” the influence of each particle over a small area using a function called a **kernel**.

### The Kernel Function
Each particle only affects other particles within a distance called `h` (the smoothing radius).

You can think of `h` like the radius of a blurry measuring circle:
- if another particle is close, it influences it a lot
- if it’s near or close to the edge of `h`, it has little influence
- if it’s farther than `h`, it doesn’t influence it at all

So every frame, the simulation looks at nearby particles and uses the kernels functions to estimate:
1) how “crowded” a particle’s area is (density)
2) how hard it should push (pressure)
3) how much it should resist different velocities (viscosity)

### 1) Density with `Poly6`
“Density” means: how much mass is packed around a particle.
`Poly6` is shaped like a smooth hill. When neighbors are close, `Poly6` gives a bigger contribution, so the particle ends up with higher density.

That’s why increasing `h` (or having more particles nearby) changes how dense the simulation feels.

### 2) Pressure with `Spiky` (the gradient)
Once we have density, SPH turns it into **pressure**. This project uses a simple rule:
- if a particle is denser than a target value (`targetDensity`), it gets positive pressure
- the bigger the density difference, the stronger the pressure
- pressure then turns into a force that pushes particles apart

The “direction” of that pushing comes from the `Spiky` kernel **gradient**:
it’s like a recipe that says which way to push so that particles move away from regions that are too crowded.

That’s why `Stiffness` (how strongly pressure grows) affects how “bouncy” and “chunky” the fluid looks.

### 3) Viscosity with `Laplacian`
Viscosity is the “mixing” / “smoothing” effect of fluid.
If two nearby particles have different velocities, viscosity tries to reduce that difference so the motion becomes smoother.

The `Laplacian` kernel is designed for this smoothing job. It helps compute a force that nudges particle velocities toward each other.

So in this project, increasing `Viscosity` makes the fluid feel slower to shear and swirl, more like thicker honey.

## Build (macOS / Linux / Windows)
Requires: a C++17 compiler + OpenGL-capable GPU/driver.

This repo uses vendored headers/sources for GLAD + ImGui, and will fetch/build GLFW during CMake configure unless you already have it.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 2
```

Run:
```bash
./build/FluidSimulation   # macOS/Linux
build\\FluidSimulation.exe  # Windows
```

## Controls
- `M`: toggle mouse camera control (mouse + WASD/QE movement)
- `Space`: pause/unpause simulation
- `R`: reset simulation
- `I`: inject an impulse pulse
- Left click: “stir” the fluid at the mouse ray/plane intersection (when not over the ImGui UI)

## GUI
Sliders for `Target Density`, `Stiffness`, `Viscosity`, `Gravity`, plus interactivity controls (`Time Scale`, impulse strength/radius).