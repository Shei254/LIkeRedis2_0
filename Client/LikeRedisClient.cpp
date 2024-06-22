//
// Created by shei on 19/06/24.
//

#include <cstring>
#include <utility>
#include <unistd.h>
#include "LikeRedisClient.h"
#include "../Networking/TCP/Errors/TCPClientException.h"
#include "../Common/LikeRedis/Client/LikeRedisClientHelpers.h"

LikeRedisClient::LikeRedisClient(const std::string& domain, std::string& port) : TCPClient(domain, port) {

}
