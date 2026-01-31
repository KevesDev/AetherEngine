add_executable(aether_editor main.cpp "layers/EditorLayer.h")
target_link_libraries(aether_editor PRIVATE aether_engine)

# AUTOMATED FIX: Copy SDL2.dll to the editor folder
add_custom_command(TARGET aether_editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2>
    $<TARGET_FILE_DIR:aether_editor>
)