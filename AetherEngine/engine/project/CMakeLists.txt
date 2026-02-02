# Add Platform files to the main engine library
target_sources(aether_engine PRIVATE
    Project.h
    Project.cpp
    ProjectSerializer.cpp
    ProjectSerializer.h
)

# Force SDL includes if needed
if (sdl2_SOURCE_DIR)
    target_include_directories(aether_engine PRIVATE "${sdl2_SOURCE_DIR}/include")
endif()