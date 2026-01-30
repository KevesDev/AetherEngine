cmake_minimum_required(VERSION 3.20)

# CRITICAL FIX: We add "C" here because SDL2 is a C library.
project(AetherEngine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- 1. DOWNLOAD DEPENDENCIES ---
include(FetchContent)

message(STATUS "Fetching SDL2...")
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG release-2.28.5
)
FetchContent_MakeAvailable(SDL2)

message(STATUS "Fetching IMGUI...")
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking # We need the docking branch for editor windows!
)
FetchContent_MakeAvailable(imgui)

# --- 2. PROJECT DEFINITION ---
project ("AetherEngine")

# --- 2. BUILD SUBDIRECTORIES ---
# This points to the inner "AetherEngine" folder
add_subdirectory(AetherEngine)