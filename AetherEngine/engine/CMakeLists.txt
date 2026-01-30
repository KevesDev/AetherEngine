project(engine)

# 1. Define the ONE shared engine library
add_library(aether_engine "")

# 2. Add subdirectories (They will append their files to aether_engine)
add_subdirectory(core)
add_subdirectory(events)
add_subdirectory(platform)

# 3. Link SDL2 to the whole engine
if (TARGET SDL2::SDL2)
    target_link_libraries(aether_engine PUBLIC SDL2::SDL2)
elseif (TARGET SDL2)
    target_link_libraries(aether_engine PUBLIC SDL2)
endif()

# 4. Setup Global Includes
target_include_directories(aether_engine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)