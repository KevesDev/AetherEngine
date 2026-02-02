add_executable(aether_enginecontent
    shaders/FlatColor.glsl
)

# Link against the Unified Engine Library
target_link_libraries(aether_enginecontent
    PRIVATE
        aether_engine
)

# Copy SDL2.dll to the same folder as the executable
add_custom_command(TARGET aether_enginecontent POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2>
    $<TARGET_FILE_DIR:aether_enginecontent>
)