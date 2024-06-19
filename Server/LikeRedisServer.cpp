//
// Created by shei on 19/06/24.
//
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include "LikeRedisServer.h"
#include "../Networking/TCP/Errors/TCPServerException.h"
#include "../Common/LikeRedis/Server/LikeRedisServerHelpers.h"


LikeRedisServer::LikeRedisServer(int port) : TCPServer(port) {
    this->handleConnections(LikeRedisServer::handleConnection);
}

void LikeRedisServer::handleConnection(int connFd) {
    LikeRedisServerHelpers::one_request(connFd);
}
