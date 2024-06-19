//
// Created by shei on 19/06/24.
//
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "TCPClient.h"
#include "Errors/TCPClientException.h"

TCPClient::TCPClient(const std::string& domain, std::string port) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        throw TCPClientException("[-] Error creating socket");
    }
    this->sock = sock;

    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* addr = nullptr;
    if (int errCode = getaddrinfo(domain.c_str(), port.c_str(), &hints, &addr)) {
        fprintf(stderr, "[-] %s\n", gai_strerror(errCode));
        throw TCPClientException("[-] Error getting server address");
    }

    if (connect(this->sock, addr->ai_addr, addr->ai_addrlen)) {
        throw TCPClientException("[-] Error connecting to server");
    }

    freeaddrinfo(addr);

    printf("[+] connection to server established");
}

void TCPClient::handleTcpConnection(void (*connection_handler)(int)) {
    connection_handler(this->sock);
    close(this->sock);
}


