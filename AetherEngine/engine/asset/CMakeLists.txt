# Engine Asset Module
set(ASSET_SOURCES
    AssetLibrary.cpp
    AssetLibrarySerializer.cpp
    AssetManager.cpp
)

# Create the object library
add_library(aether_asset OBJECT ${ASSET_SOURCES})

target_include_directories(aether_asset PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/vendor
)

target_link_libraries(aether_asset PUBLIC aether_core)