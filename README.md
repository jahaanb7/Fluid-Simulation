### Fluid Simulation

A C++/OpenGL fluid simulation using GLFW + ImGui (heavily inspired by Sebastian Lague).

## Demo-friendly UX
- The real system cursor is kept inside the app window.
- When you hover over the ImGui GUI, a small software cursor appears so you can interact comfortably.

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