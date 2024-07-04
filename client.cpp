//
// Created by shei on 19/06/24.
//

#include <string>
#include <cstring>
#include "Client/LikeRedisClient.h"
#include "Common/LikeRedis/Client/LikeRedisClientHelpers.h"
LR_REQUEST request {};
void processConnection (int fd) {
    LikeRedisClientHelpers::send_req(fd, &request);
    LikeRedisClientHelpers::recv_res(fd);
}

int main (int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "[-] expected atleast 2 arguments");
        exit(-1);
    }

    if (strncmp(argv[1], "get", strlen("get")) == 0) {
        request.command = GET;
        strncpy(request.key, argv[2], strlen(argv[2]));
    } else if (strncmp(argv[1], "set", strlen("set")) == 0) {

    } else if (strncmp(argv[1], "del", strlen("del")) == 0) {

    } else if (strncmp(argv[1], "keys", strlen("keys")) == 0) {

    }

    auto domain = std::string("0.0.0.0");
    auto port = std::string("8000");
    auto client = LikeRedisClient(domain, port);

    client.handleTcpConnection(processConnection);

    free(request.key);
    free(request.data);
    return 0;
}