# --- GLAD SETUP ---
# 1. Define the library (Static Lib)
# We point to the C file we extracted. 
# Path assumption: engine/vendor/glad/src/glad.c
add_library(glad STATIC "glad/src/glad.c" "stb_image.h")

# 2. Expose the headers
# This allows any code that uses 'glad' to simply #include <glad/glad.h>
# Path assumption: engine/vendor/glad/include
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")

# --- EXISTING VENDOR LIBS ---
# (Preserving your existing json.hpp logic)
target_sources(aether_engine PRIVATE
    json.hpp
)

target_include_directories(aether_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)