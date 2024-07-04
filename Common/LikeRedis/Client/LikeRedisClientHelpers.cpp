//
// Created by shei on 19/06/24.
//

#include <cstring>
#include <netinet/in.h>
#include "LikeRedisClientHelpers.h"
#include "../../../Networking/TCP/Errors/TCPClientException.h"
#include "../LikeRedisCommonHelpers.h"

size_t K_MAX_MSG = 1028;

int32_t LikeRedisClientHelpers::recv_res(int fd) {
    struct LR_RESPONSE *response;
    LikeRedisCommonHelpers::read_full(fd, &response, sizeof(struct LR_RESPONSE));
;

    if (response->dataLen > K_MAX_MSG) {
        throw TCPClientException("[-] Message too long");
    }


    return handle_response(response);
}

int32_t LikeRedisClientHelpers::send_req(int fd, LR_REQUEST* request) {
    if ((request->dataLen + sizeof(struct LR_REQUEST)) > K_MAX_MSG) {
        throw TCPClientException("[-] Message too long");
    }

    int dataSize = request->dataLen + sizeof(struct LR_REQUEST) + strlen(request->key);
    char wBuf[dataSize + 4];
    memcpy(wBuf, &dataSize, 4);
    memcpy(&wBuf[4], request, dataSize);

    LikeRedisCommonHelpers::write_full(fd, wBuf, dataSize + 4);
    return request->dataLen;
}

int32_t LikeRedisClientHelpers::handle_response(LR_RESPONSE* response) {
    int serialization = response->serialization;
    switch (serialization) {
        case SER_NIL:
            printf("(nil)\n");
            return 1;
        case SER_ERR:
            printf("(err) %d %.*s\n", (int)response->statusCode, (int)response->dataLen, (char *)response->data);
            return response->dataLen;
        case SER_STR:
            printf("%.*s\n", (int)response->dataLen, (char *)response->data);
            return response->dataLen;
        default:
            throw TCPClientException("Invalid Response");
    }
}
