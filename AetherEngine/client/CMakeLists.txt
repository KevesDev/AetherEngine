# CMakeLists.txt for AetherEngine client application

add_executable(aether_client
    main.cpp
)

# Link against the Unified Engine Library
target_link_libraries(aether_client
    PRIVATE
        aether_engine
)

# DO NOT CHANGE: Copy SDL2.dll to the same folder as the executable
add_custom_command(TARGET aether_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2>
    $<TARGET_FILE_DIR:aether_client>
)

# Copy Engine Content (Private Internal Data)
add_custom_command(TARGET aether_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/AetherEngine/EngineContent
    $<TARGET_FILE_DIR:aether_client>/EngineContent
)