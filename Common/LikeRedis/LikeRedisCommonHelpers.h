//
// Created by shei on 19/06/24.
//

#ifndef LIKEREDIS2_0_LIKEREDISCOMMONHELPERS_H
#define LIKEREDIS2_0_LIKEREDISCOMMONHELPERS_H

#include <cctype>
#include <cstdio>
#include <string>

const int K_MAX_KEY = 10;

enum SERIALIZATION {
    SER_NIL = 0,
    SER_ERR = 1,
    SER_STR = 2,
    SER_INT = 3,
    SER_ARR = 4
};

struct LR_RESPONSE {
    long statusCode;
    long dataLen;

    //Caller Responsible For Freeing
    void* data;
    SERIALIZATION serialization;
};

enum LR_COMMANDS {
    GET,
    SET,
    DEL,
    KEYS,
    UNKNOWN,
};

struct LR_REQUEST {
    LR_COMMANDS command = UNKNOWN;
    char key[K_MAX_KEY]{};

    void* data = nullptr;
    int dataLen = 0;
};

class LikeRedisCommonHelpers {
public:
    static void fd_set_non_blocking(int fd);
    static __int32_t read_full(int connFd, void* buf, size_t n);
    static __int32_t write_full(int connFd, const char* buf, size_t n);
};
#endif //LIKEREDIS2_0_LIKEREDISCOMMONHELPERS_H
