//
// Created by shei on 19/06/24.
//

#ifndef LIKEREDIS2_0_TCPCLIENT_H
#define LIKEREDIS2_0_TCPCLIENT_H
#include <string>

class TCPClient {
private:
    int sock;
public:
    TCPClient(const std::string& domain, std::string port);
    void handleTcpConnection (void (*connection_handler)(int sock));
    int getSocket();
};


#endif //LIKEREDIS2_0_TCPCLIENT_H
