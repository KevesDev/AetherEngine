project(engine)

find_package(OpenGL REQUIRED)

# 1. Define the ONE shared engine library
# FIX: Include the object files from the asset module
add_library(aether_engine STATIC
    $<TARGET_OBJECTS:aether_asset>
)

# 2. Add subdirectories
add_subdirectory(core)
add_subdirectory(events)
add_subdirectory(platform)
add_subdirectory(input)
add_subdirectory(vendor)
add_subdirectory(scene)
add_subdirectory(ecs)
add_subdirectory(renderer)
add_subdirectory(project)
add_subdirectory(asset)
#add_subdirectory(systems)

# 3. Link ImGui, SDL, OpenGL, GLAD, AND GLM to the whole engine
if (TARGET SDL2::SDL2)
    target_link_libraries(aether_engine PUBLIC 
        SDL2::SDL2 
        imgui 
        glad 
        OpenGL::GL
        glm::glm
    )
elseif (TARGET SDL2)
    target_link_libraries(aether_engine PUBLIC 
        SDL2 
        imgui 
        glad 
        OpenGL::GL
        glm::glm
    )
endif()

target_include_directories(aether_engine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${glm_SOURCE_DIR}
)