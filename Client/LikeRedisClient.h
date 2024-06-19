//
// Created by shei on 19/06/24.
//

#ifndef LIKEREDIS2_0_LIKEREDISCLIENT_H
#define LIKEREDIS2_0_LIKEREDISCLIENT_H
#include <string>
#include "../Networking/TCP/TCPClient.h"

class LikeRedisClient : public TCPClient {
private:
    static void handleConnection(int connFd);
public:
    explicit LikeRedisClient(const std::string& domain, std::string& port);
};


#endif //LIKEREDIS2_0_LIKEREDISCLIENT_H
