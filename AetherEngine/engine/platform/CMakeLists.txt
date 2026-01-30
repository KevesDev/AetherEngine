# Add these files to the "aether_engine" library defined in the parent folder
target_sources(aether_engine PRIVATE
    Window.cpp
    SDLWindow.cpp
)

# FORCE INCLUDE: Sometimes VS needs an explicit push to find SDL.h
if (sdl2_SOURCE_DIR)
    target_include_directories(aether_engine PRIVATE "${sdl2_SOURCE_DIR}/include")
endif()