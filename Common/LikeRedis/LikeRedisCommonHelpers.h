//
// Created by shei on 19/06/24.
//

#ifndef LIKEREDIS2_0_LIKEREDISCOMMONHELPERS_H
#define LIKEREDIS2_0_LIKEREDISCOMMONHELPERS_H

#include <cctype>
#include <cstdio>

class LikeRedisCommonHelpers {
public:
    static void fd_set_non_blocking(int fd);
    static __int32_t read_full(int connFd, char* buf, size_t n);
    static __int32_t write_full(int connFd, const char* buf, size_t n);
};


#endif //LIKEREDIS2_0_LIKEREDISCOMMONHELPERS_H
