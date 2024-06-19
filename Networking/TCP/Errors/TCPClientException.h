//
// Created by shei on 19/06/24.
//

#ifndef LIKEREDIS2_0_TCPCLIENTEXCEPTION_H
#define LIKEREDIS2_0_TCPCLIENTEXCEPTION_H
#include <string>

class TCPClientException {
private:
    std::string message;
public:
    TCPClientException(std::string exception);
    std::string getMessage();
};


#endif //LIKEREDIS2_0_TCPCLIENTEXCEPTION_H
