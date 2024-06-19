//
// Created by shei on 18/06/24.
//
#include "HTTPServer.h"

#include <utility>


void HTTPServer::createServer(std::string& domain, std::string ) {
    this->host = std::move(host);

}

HTTPServer::HTTPServer(std::string host, int port) {

}
