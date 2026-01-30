# Add Core files to the main engine library
target_sources(aether_engine PRIVATE
    EngineVersion.cpp
    Log.cpp
    Engine.cpp
    AetherTime.cpp
)

target_include_directories(aether_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
