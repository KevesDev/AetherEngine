# Core engine library
# Nothing above the engine depends on this folder,
# so it can be a reusable engine module.

# using STATIC to allow linking into both client and server builds
add_library(aether_core STATIC
    EngineVersion.cpp
    Log.cpp
    Engine.cpp
    AetherTime.cpp)


# public so that client and server can include headers
target_include_directories(aether_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}


)
