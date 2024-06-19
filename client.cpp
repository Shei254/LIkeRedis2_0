//
// Created by shei on 19/06/24.
//

#include <string>
#include "Client/LikeRedisClient.h"

int main ( ) {
    auto domain = std::string("0.0.0.0");
    auto port = std::string("8000");
    auto client = LikeRedisClient(domain, port);

    return 0;
}