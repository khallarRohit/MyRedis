cmake_minimum_required(VERSION 3.10)
project(MyRedis)

# ==========================================
# Define the Single Executable
# ==========================================
# You must list EVERY .cpp file you want to compile here.
add_executable(MyRedisServer 
    main.cpp      
    Error/error.cpp
    Ip/ipendpoint.cpp
    Socket/socket.cpp
    TCPConnection/tcpconnection.cpp 
    Server/server.cpp
    Network/network.cpp 
)

# ==========================================
# Fix Include Paths
# ==========================================
# This adds the root "MyRedis" folder to the include search path.
# This allows you to write #include "Error/error.h" inside Ip/ipendpoint.cpp
target_include_directories(MyRedisServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})