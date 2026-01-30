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

# --- 2. BUILD SUBDIRECTORIES ---
# This points to the inner "AetherEngine" folder
add_subdirectory(AetherEngine)