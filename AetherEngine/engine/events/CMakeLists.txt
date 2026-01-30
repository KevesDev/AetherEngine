# Platform-specific additions to the core engine target.
# If the core target already exists, add sources/options to it.
# Otherwise fall back to creating the static library (for standalone usage).

if (TARGET aether_core)
    target_sources(aether_core PRIVATE
        Event.cpp
        ApplicationEvent.cpp
        KeyEvent.cpp
        MouseEvent.cpp
    )

    target_include_directories(aether_core
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
else()
    # Fallback: create the target if it doesn't exist (keeps the file usable standalone).
    add_library(aether_core STATIC
        Event.cpp
        ApplicationEvent.cpp
        KeyEvent.cpp
        MouseEvent.cpp
    )

    target_include_directories(aether_core
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
