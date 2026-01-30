# Platform-specific additions to the core engine target.
# If the core target already exists, add sources/options to it.
# Otherwise fall back to creating the static library (for standalone usage).

if (TARGET aether_core)
    target_sources(aether_core PRIVATE
        Window.cpp
        SDLWindow.cpp
    )

    target_include_directories(aether_core
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Link SDL2 if the imported target exists (FetchContent above should provide SDL2::SDL2).
    if (TARGET SDL2::SDL2)
        # Use PUBLIC so the SDL include dirs are visible in the CMake model/IDE and applied when compiling aether_core.
        target_link_libraries(aether_core PUBLIC SDL2::SDL2)
    elseif (TARGET SDL2)
        target_link_libraries(aether_core PUBLIC SDL2)
    else()
        message(WARNING "SDL2 CMake target not found. Ensure SDL is fetched/installed and declared before add_subdirectory(AetherEngine/engine).")
    endif()
else()
    # Fallback: create the target if it doesn't exist (keeps the file usable standalone).
    add_library(aether_core STATIC
        Window.cpp
    )

    target_include_directories(aether_core
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# Debug: print include dirs for SDL2 and the aether_core target
if (TARGET SDL2::SDL2)
    get_target_property(_sdlincs SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES: ${_sdlincs}")
endif()

if (TARGET aether_core)
    get_target_property(_aether_incs aether_core INCLUDE_DIRECTORIES)
    get_target_property(_aether_iface aether_core INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "aether_core INCLUDE_DIRECTORIES: ${_aether_incs}")
    message(STATUS "aether_core INTERFACE_INCLUDE_DIRECTORIES: ${_aether_iface}")
endif()
