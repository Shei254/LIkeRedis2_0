//
// Created by shei on 19/06/24.
//
#include "LikeRedisServer.h"
#include "../EventLoop/EventLoop.h"
LikeRedisServer::LikeRedisServer(int port) : TCPServer(port) {
    //Initialize Event Loop
    auto eventLoop = EventLoop(this->getSocket());
}

void LikeRedisServer::handleConnection(int connFd) {
    //Initialze
}
