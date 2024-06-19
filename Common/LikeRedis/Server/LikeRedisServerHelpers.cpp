//
// Created by shei on 19/06/24.
//
#include <cstring>
#include "LikeRedisServerHelpers.h"
#include "../../../Networking/TCP/Errors/TCPServerException.h"
#include "../LikeRedisCommonHelpers.h"

static size_t K_MAX_MSG = 4096;

__int32_t LikeRedisServerHelpers::one_request(int connFd) {
    char rBuf[(4 + K_MAX_MSG + 1)];
    memset(rBuf, 0, 4 + K_MAX_MSG);
    LikeRedisCommonHelpers::read_full(connFd, rBuf, 4);

    uint32_t len = 0;
    memcpy(&len, rBuf, 4);
    if (len > K_MAX_MSG) {
        throw TCPServerException("[-] Message too long");
    }

    LikeRedisCommonHelpers::read_full(connFd, &rBuf[4], len);
    rBuf[4 + len] = '\0';
    printf("Client says: %s\n", &rBuf[4]);


    //Echo Message Using The Same Protocol
    const char reply[] = "world";
    char wBuf[4 + strlen(reply)];
    memset(wBuf, 0, 4 + strlen(reply));

    len = (uint32_t)strlen(reply);

    memcpy(wBuf, &len, 4);
    memcpy(&wBuf[4], reply, len);

    LikeRedisCommonHelpers::write_full(connFd, wBuf, 4 + len);
    return 0;
}