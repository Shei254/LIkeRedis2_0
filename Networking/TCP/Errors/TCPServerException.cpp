//
// Created by shei on 18/06/24.
//

#include "TCPServerException.h"

TCPServerException::TCPServerException(std::string exception) {
    this->message = std::move(exception);
}

std::string TCPServerException::getMessage() {
    return this->message;
}
