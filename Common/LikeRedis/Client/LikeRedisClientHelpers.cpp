//
// Created by shei on 19/06/24.
//

#include <cstring>
#include <netinet/in.h>
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

    return handle_response(reinterpret_cast<const uint8_t *>(&rBuf[4]), len);
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

int32_t LikeRedisClientHelpers::handle_response(const uint8_t *data, size_t size) {
    if (size < 1) {
        throw TCPClientException("Invalid Response");
    }

    int serialization = data[0] - '0';
    size_t arrBytes;
    char errCodeStr[4 + 1]{};
    char lenStr[4 + 1]{};
    long errLen, strLen = 0;
    switch (serialization) {
        case SER_NIL:
            printf("(nil)\n");
            return 1;
        case SER_ERR:
            if (size < 9) {
                printf("Invalid Response\n");
                return -1;
            }

            memcpy(&errCodeStr, (unsigned char *)&data[1], 4);
            memcpy(&lenStr, (unsigned char *)&data[5], 4);

            errLen = strtol(lenStr, nullptr, 10);
            if (size < 6 + errLen) {
                printf("Invalid Response\n");
                return -1;
            }
            printf("(err) %s %.*s\n", errCodeStr, (int)errLen, &data[7]);
            return 9 + errLen;
        case SER_STR:
            if (size < 9) {
                throw TCPClientException("Invalid Response");
            }
            memcpy(lenStr, &data[5], 4);
            strLen = strtol(lenStr, nullptr, 10);
            if (size < 9 + strLen) {
                throw TCPClientException("Invalid Response");
            }
            printf("%.*s\n", (int)strLen, &data[9]);
            return 9 + strLen;
        case SER_ARR:
            if (size < 5) {
                throw TCPClientException("Invalid Response");
            }
            uint32_t arrLen;
            memcpy(&arrLen, &data[1], 4);
            printf("(arr) len=%u\n", arrLen);

            arrBytes = 5;
            for (uint32_t i = 0; i < arrLen; i++) {
                int32_t rv = handle_response(&data[arrBytes], size - arrBytes);
                if (rv < 0) {
                    return rv;
                }
                arrBytes += rv;
            }
            printf("(arr) end\n");
            return arrBytes;
        default:
            throw TCPClientException("Invalid Response");
    }
}
