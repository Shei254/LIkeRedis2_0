//
// Created by shei on 18/06/24.
//

#ifndef LIKEREDIS2_0_TCPSERVER_H
#define LIKEREDIS2_0_TCPSERVER_H
class TCPServer {
private:
    int sock;
    int port;
    void createServer();
public:
    TCPServer (int port);
    int getSocket();

    //Caller is responsible for closing client socket
    void handleConnections(void (*accept_connection)(int connFd));
};


#endif //LIKEREDIS2_0_TCPSERVER_H
