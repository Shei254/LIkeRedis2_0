//
// Created by shei on 19/06/24.
//

#include <cassert>
#include <csignal>
#include "LikeRedisCommonHelpers.h"
#include "../../Networking/TCP/Errors/TCPClientException.h"

__int32_t LikeRedisCommonHelpers::write_full(int connFd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(connFd, (void *)buf, n);
        if (rv <= 0) {
            throw TCPClientException("[-] Error writing data to server");
        }
        n -= (size_t)rv;
        buf += (size_t)rv;
    }
    printf("[+] message sent successfully\n");
    return 0;
}

__int32_t LikeRedisCommonHelpers::read_full(int connFd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(connFd, buf, n);
        if (rv <= 0) {
            throw TCPClientException("[-] Unexpected EOF");
        }
        n -= (size_t)rv;
        buf += (size_t)rv;
    }
    return 0;
}
