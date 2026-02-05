project(engine_network)

add_library(aether_network OBJECT
    NetworkTransport.h
    UdpTransport.h
    UdpTransport.cpp
)

target_include_directories(aether_network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

