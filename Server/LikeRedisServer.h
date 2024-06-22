//
// Created by shei on 19/06/24.
//

#ifndef LIKEREDIS2_0_LIKEREDISSERVER_H
#define LIKEREDIS2_0_LIKEREDISSERVER_H

#include <unordered_map>
#include "../Networking/TCP/TCPServer.h"
#include "../EventLoop/EventLoop.h"

class LikeRedisServer : public TCPServer {
private:
    static void handleConnection(int connFd);
public:
    explicit LikeRedisServer(int port);
};


#endif //LIKEREDIS2_0_LIKEREDISSERVER_H
