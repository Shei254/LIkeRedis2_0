//
// Created by shei on 19/06/24.
//

#include <string>
#include "Client/LikeRedisClient.h"
#include "Common/LikeRedis/Client/LikeRedisClientHelpers.h"

std::vector<std::string> cmd;
void processConnection (int fd) {
    LikeRedisClientHelpers::send_req(fd, cmd);
    LikeRedisClientHelpers::recv_res(fd);
}

int main (int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        cmd.emplace_back(argv[i]);
    }


    auto domain = std::string("0.0.0.0");
    auto port = std::string("8000");
    auto client = LikeRedisClient(domain, port);

    client.handleTcpConnection(processConnection);
    return 0;
}