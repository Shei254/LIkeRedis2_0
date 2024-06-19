#include <iostream>
#include "Server/LikeRedisServer.h"

int main() {
    auto server = LikeRedisServer(8000);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
