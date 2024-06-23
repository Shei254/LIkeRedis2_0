//
// Created by shei on 19/06/24.
//

#ifndef LIKEREDIS2_0_LIKEREDISCLIENTHELPERS_H
#define LIKEREDIS2_0_LIKEREDISCLIENTHELPERS_H


#include <cstdint>
#include <vector>
#include <string>

class LikeRedisClientHelpers {
public:
    static int32_t send_req (int fd, const std::vector<std::string> &cmd);
    static int32_t handle_response(const uint8_t* );
    static int32_t recv_res(int fd);
};


#endif //LIKEREDIS2_0_LIKEREDISCLIENTHELPERS_H
