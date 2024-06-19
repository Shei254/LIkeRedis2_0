//
// Created by shei on 19/06/24.
//

#include "TCPClientException.h"

#include <utility>
TCPClientException::TCPClientException(std::string exception) {
    this->message = std::move(exception);
}

std::string TCPClientException::getMessage() {
    return this->message;
}
