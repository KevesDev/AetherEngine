cmake_minimum_required(VERSION 3.20)
project(AetherEngine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define AETHER_DEBUG macro only for Debug builds
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions(AETHER_DEBUG)
endif()

# --- 1. DOWNLOAD DEPENDENCIES ---
include(FetchContent)

# 1.1 Fetch SDL2
message(STATUS "Fetching SDL2...")
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG release-2.28.5
)
FetchContent_MakeAvailable(SDL2)

# 1.2 Fetch ImGui (AND BUILD IT MANUALLY)
message(STATUS "Fetching ImGui...")
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
  
  # MANUALLY CREATE THE LIBRARY TARGET
  # This tells CMake: "Take these C++ files and build them into imgui.lib"
  add_library(imgui STATIC
      ${imgui_SOURCE_DIR}/imgui.cpp
      ${imgui_SOURCE_DIR}/imgui_demo.cpp
      ${imgui_SOURCE_DIR}/imgui_draw.cpp
      ${imgui_SOURCE_DIR}/imgui_tables.cpp
      ${imgui_SOURCE_DIR}/imgui_widgets.cpp
      # We also include the backends we need
      ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
      ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  )

  # Tell the rest of the project where to find headers
  target_include_directories(imgui PUBLIC 
      ${imgui_SOURCE_DIR} 
      ${imgui_SOURCE_DIR}/backends
  )

  # ImGui backends need access to SDL headers
  target_link_libraries(imgui PUBLIC SDL2::SDL2)
endif()

# --- 2. BUILD SUBDIRECTORIES ---
add_subdirectory(AetherEngine)