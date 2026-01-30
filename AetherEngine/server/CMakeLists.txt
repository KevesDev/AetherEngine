# CMakeLists.txt for AetherEngine project server application

# Build the server executable
add_executable(aether_server
    main.cpp
)

# Link against the core engine library
target_link_libraries(aether_server
    PRIVATE
        aether_core
)

