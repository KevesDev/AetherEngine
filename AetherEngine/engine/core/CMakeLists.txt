# Add Core files to the main engine library
target_sources(aether_engine PRIVATE
    EngineVersion.cpp
    Log.cpp
    Engine.cpp
    AetherTime.cpp
 "Layers/Layer.h" "Layers/Layer.cpp" "Layers/LayerStack.h" "Layers/LayerStack.cpp" "Config.h" "Config.cpp" "Layers/ImGuiLayer.h" "Layers/ImGuiLayer.cpp")

target_include_directories(aether_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
