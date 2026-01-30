# CMakeLists.txt for Aether Editor

# Build the editor executable
add_executable(aether_editor
    main.cpp
)

# Link against the core engine library
target_link_libraries(aether_editor
    PRIVATE
        aether_core
)
