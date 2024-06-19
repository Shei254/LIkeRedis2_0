//
// Created by shei on 19/06/24.
//

#ifndef LIKEREDIS2_0_LIKEREDISCLIENTHELPERS_H
#define LIKEREDIS2_0_LIKEREDISCLIENTHELPERS_H


#include <cstdint>

class LikeRedisClientHelpers {
public:
    static int32_t query(int connFd, const char* text);
};


#endif //LIKEREDIS2_0_LIKEREDISCLIENTHELPERS_H
