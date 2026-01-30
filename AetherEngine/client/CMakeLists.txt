# CMakeLists.txt for AetherEngine client application

add_executable(aether_client
    main.cpp
)

# Link against the Unified Engine Library
target_link_libraries(aether_client
    PRIVATE
        aether_engine
)
