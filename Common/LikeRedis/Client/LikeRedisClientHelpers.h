//
// Created by shei on 19/06/24.
//

#ifndef LIKEREDIS2_0_LIKEREDISCLIENTHELPERS_H
#define LIKEREDIS2_0_LIKEREDISCLIENTHELPERS_H


#include <cstdint>
#include <vector>
#include <string>
#include "../LikeRedisCommonHelpers.h"

class LikeRedisClientHelpers {
public:
    static int32_t send_req (int fd, LR_REQUEST* request);
    static int32_t handle_response(LR_RESPONSE *response);
    static int32_t recv_res(int fd);
};


#endif //LIKEREDIS2_0_LIKEREDISCLIENTHELPERS_H
