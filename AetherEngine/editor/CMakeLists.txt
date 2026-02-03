add_executable(aether_editor main.cpp "layers/EditorLayer.h" "layers/EditorLayer.cpp" "panels/EditorPanel.h" "panels/SceneHierarchyPanel.cpp" "layers/ProjectHubLayer.h" "layers/ProjectHubLayer.cpp" "panels/InspectorPanel.h" "panels/InspectorPanel.cpp" "EditorCamera.h" "EditorCamera.cpp" "panels/ContentBrowserPanel.h" "panels/ContentBrowserPanel.cpp" "EditorResources.h" "EditorResources.cpp" "panels/AssetEditorPanel.h" "panels/AssetEditorPanel.cpp" "panels/TextureViewerPanel.h" "panels/TextureViewerPanel.cpp" "panels/FileBrowser.h" "panels/FileBrowser.cpp" "commands/EditorCommand.h" "commands/CommandHistory.cpp" "commands/DeleteAssetCommand.h" "commands/DeleteAssetCommand.cpp")
target_link_libraries(aether_editor PRIVATE aether_engine)

# DO NOT CHANGE: Copy SDL2.dll to the editor folder
add_custom_command(TARGET aether_editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2>
    $<TARGET_FILE_DIR:aether_editor>
)

# Copy Engine Content (Private Internal Data)
add_custom_command(TARGET aether_editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/AetherEngine/EngineContent
    $<TARGET_FILE_DIR:aether_editor>/EngineContent
)