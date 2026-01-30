project(engine)
project(engine)

# 1. Define the ONE shared engine library
add_library(aether_engine "")

# 2. Add subdirectories (They will append their files to aether_engine)
add_subdirectory(core)
add_subdirectory(events)
add_subdirectory(platform)

# 3. Link ImGui and SDL to the whole engine
if (TARGET SDL2::SDL2)
    target_link_libraries(aether_engine PUBLIC SDL2::SDL2 imgui)
elseif (TARGET SDL2)
    target_link_libraries(aether_engine PUBLIC SDL2 imgui)
endif()

target_include_directories(aether_engine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${imgui_SOURCE_DIR} # Allows you to do #include <imgui.h>
    ${imgui_SOURCE_DIR}/backends # Allows you to do #include <backends/imgui_impl_sdl2.h>
)