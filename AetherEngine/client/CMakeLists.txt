# CMakeLists.txt for AetherEngine client application

# Build the client executable
add_executable(aether_client
    main.cpp
)

# Link against the core engine library
target_link_libraries(aether_client
    PRIVATE
        aether_core
)
