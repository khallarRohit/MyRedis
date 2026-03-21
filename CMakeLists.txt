cmake_minimum_required(VERSION 3.10.0)
project(MyRedis VERSION 0.1.0 LANGUAGES C CXX)

set(COMMON_SOURCES 
    Ip/ipendpoint.cpp 
    Error/WSAexception.cpp
    Socket/socket.cpp
    Network/network.cpp
    TCPConnection/tcpconnection.cpp
)

add_library(RedisCommon STATIC ${COMMON_SOURCES})

target_link_libraries(RedisCommon PUBLIC ws2_32)

target_include_directories(RedisCommon PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(RedisServer
    server_main.cpp      # The main() for the server
    Server/server.cpp    # The Server class logic
)

target_link_libraries(RedisServer PRIVATE RedisCommon)

add_executable(RedisClient
    client_main.cpp      # A NEW file: The main() for the client
    Client/client.cpp    # A NEW file: The Client class logic
)

target_link_libraries(RedisClient PRIVATE RedisCommon)

include(CTest)
enable_testing()

