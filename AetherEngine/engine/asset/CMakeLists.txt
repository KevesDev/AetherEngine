# Engine Asset Module
set(ASSET_SOURCES
    AssetLibrary.cpp
    AssetLibrarySerializer.cpp
    AssetManager.cpp
    AssetMetadata.h
)

# Create the object library without duplicate source entries
add_library(aether_asset OBJECT ${ASSET_SOURCES})

# Use PROJECT_SOURCE_DIR (defined in engine/CMakeLists.txt) to get the correct path to 'engine'
target_include_directories(aether_asset PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/vendor
)

# Link dependencies
# 1. We link 'glad' so that aether_asset inherits the include path for glad.h
# 2. We DO NOT link 'aether_core' because it is not a standalone library target
target_link_libraries(aether_asset PUBLIC glad)