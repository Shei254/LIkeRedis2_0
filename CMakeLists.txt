cmake_minimum_required(VERSION 3.27)
project(LikeRedis2_0)

set(CMAKE_CXX_STANDARD 17)

add_executable(LikeRedisServer2_0 server.cpp
        Networking/TCP/TCPServer.h
        Networking/TCP/TCPServer.cpp
        Networking/TCP/Errors/TCPClientException.h
        Networking/TCP/Errors/TCPClientException.cpp
        Networking/TCP/Errors/TCPServerException.h
        Networking/TCP/Errors/TCPServerException.cpp
        Server/LikeRedisServer.h
        Server/LikeRedisServer.cpp
        Common/LikeRedis/LikeRedisCommonHelpers.h
        Common/LikeRedis/LikeRedisCommonHelpers.cpp
        EventLoop/EventLoop.h
        EventLoop/EventLoop.cpp
)

add_executable(LikeRedisClient2_0 client.cpp
        Networking/TCP/TCPClient.h
        Networking/TCP/TCPClient.cpp
        Networking/TCP/Errors/TCPClientException.h
        Networking/TCP/Errors/TCPClientException.cpp
        Client/LikeRedisClient.h
        Client/LikeRedisClient.cpp
        Common/LikeRedis/LikeRedisCommonHelpers.h
        Common/LikeRedis/LikeRedisCommonHelpers.cpp
        Common/LikeRedis/Client/LikeRedisClientHelpers.h
        Common/LikeRedis/Client/LikeRedisClientHelpers.cpp
)