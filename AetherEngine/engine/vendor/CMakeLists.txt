# Add Event files to the main engine library
target_sources(aether_engine PRIVATE
    json.hpp
)

target_include_directories(aether_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)