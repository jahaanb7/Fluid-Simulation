cmake_minimum_required(VERSION 3.19)
project(FluidSimulation LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ── ImGui (shared: native + web) ─────────────────────────────────────────────
set(FLUIDSIM_IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/imgui")
set(FLUIDSIM_IMGUI_BACKENDS_DIR "${FLUIDSIM_IMGUI_DIR}/backends")

if(NOT EXISTS "${FLUIDSIM_IMGUI_DIR}/imgui.cpp")
  FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
  )
  FetchContent_MakeAvailable(imgui)
  set(FLUIDSIM_IMGUI_DIR        "${imgui_SOURCE_DIR}")
  set(FLUIDSIM_IMGUI_BACKENDS_DIR "${FLUIDSIM_IMGUI_DIR}/backends")
endif()

if(NOT EXISTS "${FLUIDSIM_IMGUI_DIR}/imgui.cpp")
  message(FATAL_ERROR "Dear ImGui sources not found.")
endif()

set(IMGUI_SOURCES
  "${FLUIDSIM_IMGUI_DIR}/imgui.cpp"
  "${FLUIDSIM_IMGUI_DIR}/imgui_draw.cpp"
  "${FLUIDSIM_IMGUI_DIR}/imgui_tables.cpp"
  "${FLUIDSIM_IMGUI_DIR}/imgui_widgets.cpp"
  "${FLUIDSIM_IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp"
  "${FLUIDSIM_IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp"
)

# ─────────────────────────────────────────────────────────────────────────────
# EMSCRIPTEN  (WebAssembly / GitHub Pages)
# ─────────────────────────────────────────────────────────────────────────────
if(EMSCRIPTEN)

  # The browser build compiles the same SPH code (src/library) without OpenGL or ImGui.
  # web/fluid-view.js draws it with WebGL2. Output: web/dist/fluid.mjs (wasm embedded).
  add_executable(fluid_web web/fluid_wasm.cpp)

  set_target_properties(fluid_web PROPERTIES
    OUTPUT_NAME "fluid"
    SUFFIX      ".mjs"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/web/dist"
  )

  target_include_directories(fluid_web PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include"
  )

  target_compile_definitions(fluid_web PRIVATE FLUIDSIM_NO_GL)
  target_compile_options(fluid_web PRIVATE -O3)

  target_link_options(fluid_web PRIVATE
    -O3
    -sMODULARIZE=1
    -sEXPORT_ES6=1
    -sENVIRONMENT=web,worker
    -sALLOW_MEMORY_GROWTH=1
    -sSINGLE_FILE=1
    -sEXPORTED_RUNTIME_METHODS=HEAPF32
    "-sEXPORTED_FUNCTIONS=_fs_reset,_fs_step,_fs_count,_fs_data,_fs_box,_fs_set,_fs_get,_fs_impulse,_fs_stir,_fs_density,_malloc"
  )

# ─────────────────────────────────────────────────────────────────────────────
# NATIVE  (Linux / macOS / Windows)
# ─────────────────────────────────────────────────────────────────────────────
else()

  option(FLUIDSIM_FETCH_GLFW "Fetch/build GLFW from source" ON)

  find_package(OpenGL REQUIRED)
  find_package(OpenMP QUIET)

  # GLFW ──────────────────────────────────────────────────────────────────────
  set(GLFW_TARGET "")

  if(FLUIDSIM_FETCH_GLFW)
    FetchContent_Declare(glfw
      GIT_REPOSITORY https://github.com/glfw/glfw.git
      GIT_TAG        3.4
    )
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)

    if(TARGET glfw)
      set(GLFW_TARGET glfw)
    elseif(TARGET glfw3)
      set(GLFW_TARGET glfw3)
    endif()
  endif()

  if(GLFW_TARGET STREQUAL "")
    message(FATAL_ERROR "GLFW not found. Enable FLUIDSIM_FETCH_GLFW=ON.")
  endif()

  # Executable ────────────────────────────────────────────────────────────────
  add_executable(FluidSimulation
    src/main.cpp
    src/glad.c
    ${IMGUI_SOURCES}
  )

  target_include_directories(FluidSimulation PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include"
    "${FLUIDSIM_IMGUI_DIR}"
    "${FLUIDSIM_IMGUI_BACKENDS_DIR}"
  )

  target_link_libraries(FluidSimulation PRIVATE
    ${GLFW_TARGET}
    OpenGL::GL
  )

  if(OpenMP_CXX_FOUND)
    target_link_libraries(FluidSimulation PRIVATE OpenMP::OpenMP_CXX)
  endif()

  if(APPLE)
    target_link_libraries(FluidSimulation PRIVATE
      "-framework Cocoa"
      "-framework OpenGL"
      "-framework IOKit"
      "-framework CoreVideo"
    )
  endif()

  if(NOT MSVC)
    target_compile_options(FluidSimulation PRIVATE -Wall -Wextra -Wpedantic)
  endif()

endif()
