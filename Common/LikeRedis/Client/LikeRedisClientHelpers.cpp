//
// Created by shei on 19/06/24.
//

#include <cstring>
#include "LikeRedisClientHelpers.h"
#include "../../../Networking/TCP/Errors/TCPClientException.h"
#include "../LikeRedisCommonHelpers.h"

size_t K_MAX_MSG = 1028;

int32_t LikeRedisClientHelpers::recv_res(int fd) {
    char rBuf[4 + K_MAX_MSG + 1];
    LikeRedisCommonHelpers::read_full(fd, rBuf, 4);

    int len;
    memcpy(&len, rBuf, 4);
    if (len > K_MAX_MSG) {
        throw TCPClientException("[-] Message too long");
    }

    LikeRedisCommonHelpers::read_full(fd, &rBuf[4], len);
    if (len < 4) {
        throw TCPClientException("[-] Invalid Response");
    }

    uint32_t resCode = 0;
    memcpy(&resCode, &rBuf[4], 4);
    printf("Server says: {%u} %.*s\n", resCode, len - 4, &rBuf[8]);
    return 0;
}

int32_t LikeRedisClientHelpers::send_req(int fd, const std::vector<std::string> &cmd) {
    auto len = 4;

    for (const std::string &s : cmd) {
        len += 4 + s.size();
    }

    if (len > K_MAX_MSG) {
        throw TCPClientException("[-] Message too long");
    }

    char wBuf[4 + K_MAX_MSG];
    memcpy(&wBuf[0], &len, 4);

    uint32_t n = cmd.size();
    memcpy(&wBuf[4], &n, 4);

    size_t curr = 8;
    for (const std::string &s : cmd) {
        uint32_t  p = (uint32_t)s.size();
        memcpy(&wBuf[curr], &p, 4);
        memcpy(&wBuf[curr + 4], s.data(), s.size());
        curr += 4 + s.size();
    }
    LikeRedisCommonHelpers::write_full(fd, wBuf, 4 + len);
    return len;
}
