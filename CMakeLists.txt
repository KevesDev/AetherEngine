include(FetchContent)

cmake_minimum_required(VERSION 3.21)

# Aether Engine
# A modular game engine with client-server architecture and an integrated editor.
project(AetherEngine LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch SDL2 library BEFORE adding subdirectories so subprojects can link against it.
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG release-2.28.5  # Use a fixed tag for stability
)
FetchContent_MakeAvailable(SDL2)

# Include subdirectories
add_subdirectory(AetherEngine/engine)
add_subdirectory(AetherEngine/client)
add_subdirectory(AetherEngine/server)
add_subdirectory(AetherEngine/editor)
