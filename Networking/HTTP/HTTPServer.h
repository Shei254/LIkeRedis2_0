//
// Created by shei on 18/06/24.
//
#ifndef LIKEREDIS2_0_HTTPSERVER_H
#define LIKEREDIS2_0_HTTPSERVER_H
#include <string>

class HTTPServer {
private:
    int sock;
    std::string host;
    void createServer(std::string host, int port);
public:
    HTTPServer(std::string host, int port);
};
#endif //LIKEREDIS2_0_HTTPSERVER_H
