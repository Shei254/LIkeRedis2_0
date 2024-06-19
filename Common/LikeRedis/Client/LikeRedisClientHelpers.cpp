//
// Created by shei on 19/06/24.
//

#include <cstring>
#include "LikeRedisClientHelpers.h"
#include "../../../Networking/TCP/Errors/TCPClientException.h"
#include "../LikeRedisCommonHelpers.h"

static size_t K_MAX_MSG = 4096;
int32_t LikeRedisClientHelpers::query(int connFd, const char *text) {
    uint32_t len = (uint32_t)strlen(text);
    if (len > K_MAX_MSG) {
        throw TCPClientException("[-] Message too long");
    }

    char wBuf[4 + K_MAX_MSG];
    memset(wBuf, 0, 4 + K_MAX_MSG);
    memcpy(wBuf, &len, 4);
    memcpy(&wBuf[4], text, len);

    LikeRedisCommonHelpers::write_full(connFd, wBuf, 4 + len);

    char rBuf[4 + K_MAX_MSG + 1];
    memset(wBuf, 0, 4 + K_MAX_MSG + 1);
    LikeRedisCommonHelpers::read_full(connFd, rBuf, 4);

    memcpy(&len, rBuf, 4);
    if (len > K_MAX_MSG) {
        throw TCPClientException("[-] Message too long");
    }

    LikeRedisCommonHelpers::read_full(connFd, &rBuf[4], len);

    rBuf[4 + len] = '\0';
    printf("Server says: %s\n", &rBuf[4]);
    return 0;
}
