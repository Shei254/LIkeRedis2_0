//
// Created by shei on 18/06/24.
//

#ifndef LIKEREDIS2_0_TCPSERVEREXCEPTION_H
#define LIKEREDIS2_0_TCPSERVEREXCEPTION_H
#include <string>

class TCPServerException {
private:
    std::string message;
public:
    TCPServerException(std::string exception);
    std::string getMessage();
};


#endif //LIKEREDIS2_0_TCPSERVEREXCEPTION_H
