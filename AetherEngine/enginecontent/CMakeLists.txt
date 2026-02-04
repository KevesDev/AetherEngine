add_executable(aether_EngineContent
    shaders/FlatColor.glsl
    shaders/Texture.glsl
)

# Link against the Unified Engine Library
target_link_libraries(aether_EngineContent
    PRIVATE
        aether_engine
)

# Copy SDL2.dll to the same folder as the executable
add_custom_command(TARGET aether_EngineContent POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2>
    $<TARGET_FILE_DIR:aether_EngineContent>
)