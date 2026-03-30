cmake_minimum_required(VERSION 3.19)
project(FluidSimulation LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(FLUIDSIM_FETCH_GLFW "Fetch/build GLFW from source if not found" ON)

find_package(OpenGL REQUIRED)

find_package(OpenMP QUIET)

set(FLUIDSIM_IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/imgui")
set(FLUIDSIM_IMGUI_BACKENDS_DIR "${FLUIDSIM_IMGUI_DIR}/backends")

# GLFW: prefer system, but fetch if missing.
set(GLFW_TARGET "")

# macOS: only use vendored GLFW dylib as an offline fallback.
if(APPLE AND (NOT FLUIDSIM_FETCH_GLFW))
  set(FLUIDSIM_LOCAL_GLFW_LIB "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/library/libglfw.dylib")
  if(EXISTS "${FLUIDSIM_LOCAL_GLFW_LIB}")
    add_library(glfw_local SHARED IMPORTED)
    set_target_properties(glfw_local PROPERTIES IMPORTED_LOCATION "${FLUIDSIM_LOCAL_GLFW_LIB}")
    set(GLFW_TARGET glfw_local)
  endif()
endif()

if(TARGET glfw)
  set(GLFW_TARGET glfw)
elseif(TARGET glfw3)
  set(GLFW_TARGET glfw3)
elseif(TARGET GLFW::GLFW)
  set(GLFW_TARGET GLFW::GLFW)
endif()

if(GLFW_TARGET STREQUAL "" AND FLUIDSIM_FETCH_GLFW)
  include(FetchContent)
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
  )

  # Keep CI/builds lightweight.
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(glfw)

  if(TARGET glfw)
    set(GLFW_TARGET glfw)
  elseif(TARGET glfw3)
    set(GLFW_TARGET glfw3)
  endif()
endif()

if(GLFW_TARGET STREQUAL "")
  message(FATAL_ERROR "GLFW not found. Install GLFW dev packages or enable FLUIDSIM_FETCH_GLFW.")
endif()

add_executable(FluidSimulation
  src/main.cpp
  src/glad.c
  "${FLUIDSIM_IMGUI_DIR}/imgui.cpp"
  "${FLUIDSIM_IMGUI_DIR}/imgui_draw.cpp"
  "${FLUIDSIM_IMGUI_DIR}/imgui_tables.cpp"
  "${FLUIDSIM_IMGUI_DIR}/imgui_widgets.cpp"
  "${FLUIDSIM_IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp"
  "${FLUIDSIM_IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp"
)

target_include_directories(FluidSimulation PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/imgui"
)

target_link_libraries(FluidSimulation PRIVATE
  "${GLFW_TARGET}"
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

# Sensible warnings (non-fatal).
if(NOT MSVC)
  target_compile_options(FluidSimulation PRIVATE -Wall -Wextra -Wpedantic)
endif()

