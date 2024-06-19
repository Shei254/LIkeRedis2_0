//
// Created by shei on 19/06/24.
//

#include <cstring>
#include <utility>
#include <unistd.h>
#include "LikeRedisClient.h"
#include "../Networking/TCP/Errors/TCPClientException.h"
#include "../Common/LikeRedis/Client/LikeRedisClientHelpers.h"

LikeRedisClient::LikeRedisClient(const std::string& domain, std::string& port) : TCPClient(domain, port) {
    this->handleTcpConnection(handleConnection);
}

void LikeRedisClient::handleConnection(int connFd) {
    LikeRedisClientHelpers::query(connFd, "hello1");
    LikeRedisClientHelpers::query(connFd, "hello2");
    LikeRedisClientHelpers::query(connFd, "hello3");
}
