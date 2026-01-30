project(engine)

# 1. Define the library
# We create a static library named "aether_engine" to avoid naming conflicts
add_library(aether_engine "")

# 2. Add Source Files via Subdirectories
add_subdirectory(core)
add_subdirectory(events)
add_subdirectory(platform)

# 3. LINK SDL2 (CRITICAL FIX)
# This command tells Visual Studio: "aether_engine uses SDL2"
if (TARGET SDL2::SDL2)
    target_link_libraries(aether_engine PUBLIC SDL2::SDL2)
elseif (TARGET SDL2)
    target_link_libraries(aether_engine PUBLIC SDL2)
endif()

# 4. Setup Include Directories
target_include_directories(aether_engine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)