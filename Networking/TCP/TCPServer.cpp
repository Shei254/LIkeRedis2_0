//
// Created by shei on 18/06/24.
//

#include "TCPServer.h"
#include "Errors/TCPServerException.h"
#include "../../Common/LikeRedis/LikeRedisCommonHelpers.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

TCPServer::TCPServer(int port) {
    this->port = port;
    this->createServer();
}

void TCPServer::createServer() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw TCPServerException("[-] Error creating socket");
    }
    this->sock = sock;

    //Make Ports Reusable
    int val = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    //Set Socket To Non Blocking
    LikeRedisCommonHelpers::fd_set_non_blocking(sock);

    //Bing Socket To Address
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);

    //Bind to 0.0.0.0
    addr.sin_addr.s_addr = ntohl(0);

    int rv = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (rv) {
        close(this->sock);
        throw TCPServerException("[-] Error binding to socket");
    }

    //Listen for incoming connections
    rv = listen(this->sock, 10);
    if (rv) {
        close(this->sock);
        throw TCPServerException("[-] Error listening to new connections");
    }
}

int TCPServer::getSocket() {
    return this->sock;
}

void TCPServer::handleConnections(void (*accept_connection)(int)) {
    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);

        int connFd = accept(this->sock, (struct sockaddr*)&client_addr, &socklen);
        if (connFd < 0) {
            printf("[-] error accepting new connection");
            continue;
        }

        accept_connection(connFd);
        close(connFd);


        printf("[+] Client connection closed successfully\n");
    }
}


