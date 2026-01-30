add_executable(aether_server main.cpp)
target_link_libraries(aether_server PRIVATE aether_engine)

# AUTOMATED FIX: Copy SDL2.dll to the server folder
add_custom_command(TARGET aether_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2>
    $<TARGET_FILE_DIR:aether_server>
)
