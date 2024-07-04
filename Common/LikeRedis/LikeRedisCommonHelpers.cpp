//
// Created by shei on 19/06/24.
//

#include <cassert>
#include <csignal>
#include <unistd.h>
#include <fcntl.h>

#include "LikeRedisCommonHelpers.h"
#include "../../Networking/TCP/Errors/TCPClientException.h"



__int32_t LikeRedisCommonHelpers::write_full(int connFd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(connFd, (void *)buf, n);
        if (rv < 0) {
            perror("[-] Error writing data to server");
            throw TCPClientException("[-] Error writing data to server");
        }
        n -= (size_t)rv;
        buf += (size_t)rv;
    }
    printf("[+] message sent successfully\n");
    return 0;
}

__int32_t LikeRedisCommonHelpers::read_full(int connFd, void *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(connFd, buf, n);
        if (rv < 0) {
            perror("[-] Unexpected EOF");
            throw TCPClientException("[-] Unexpected EOF");
        } else if (rv == 0) {
            printf("[+] Successful EOF\n");
            break;
        }
        n -= (size_t)rv;
        buf = (unsigned char*)buf + (size_t)rv;
    }
    return 0;
}

void LikeRedisCommonHelpers::fd_set_non_blocking(int fd) {
    errno = 0;
    int flags = fcntl(fd, F_GETFL, 0);
    if (errno) {
        perror("[-] Error getting descriptor flags");
        throw TCPClientException("[-] Error getting descriptor flag");
    }

    flags |= O_NONBLOCK;

    errno = 0;
    (void) fcntl(fd, F_SETFL, flags);

    if (errno) {
        perror("[-] Error setting flags to descriptor");
        exit(-1);
    }
}
