# Add Event files to the main engine library
target_sources(aether_engine PRIVATE
    Components.h
    Entity.h
    Registry.h
)

target_include_directories(aether_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)