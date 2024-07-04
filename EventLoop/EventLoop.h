//
// Created by shei on 20/06/24.
//

#ifndef LIKEREDIS2_0_EVENTLOOP_H
#define LIKEREDIS2_0_EVENTLOOP_H

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include "../Common/LikeRedis/LikeRedisCommonHelpers.h"

static int k_max_msg = 1048;
enum {
    STATE_REQ = 0,
    STATE_RES = 1,
    STATE_END = 2
};

enum {
    RES_OK = 0,
    RES_ERR = 1,
    RES_NX = 2
};

struct Conn {
    int fd = -1;
    uint32_t state = 0;
    size_t r_buf_size = 0;
    uint8_t r_buf[1028]{};

    size_t w_buf_size = 0;
    size_t w_buf_sent = 0;
    uint8_t w_buf[1028]{};
};


class EventLoop {
public:

private:
    int epollFd;
    int maxEvents = 10;

    static void state_req(Conn* conn);
    static void state_res(Conn* conn);
    static void connection_io(Conn* conn);
    static bool try_fill_buffer (Conn* conn);
    static bool try_one_request(Conn* conn);
    static bool try_flush_buffer(Conn* conn);
    static void connection_put(Conn* conn, std::vector<Conn*> &connections);
    static int32_t parse_request(const uint8_t* data, size_t len, std::vector<std::string> &out);
    static int32_t accept_new_connection(int fd, std::vector<Conn*> &connections);
    static void handle_request(LR_REQUEST* req, LR_RESPONSE &response);
    static void handle_get_request(LR_REQUEST *request, LR_RESPONSE &response);
    static void handle_set_request(LR_REQUEST *request, LR_RESPONSE &response);
    static void handle_del_request(LR_REQUEST *request, LR_RESPONSE &response);
    static void handle_keys_request(LR_REQUEST *request, LR_RESPONSE &response);

    static void nil_response (LR_RESPONSE &response);
    static void str_response (LR_RESPONSE &response, const std::string &val);
    static void int_response(LR_RESPONSE &response, int64_t val);
    static void err_response(LR_RESPONSE &response, int32_t code, const std::string &msg);
    static void arr_response(LR_RESPONSE &response, uint32_t n);
public:
    explicit EventLoop(int serverFd);
};


#endif //LIKEREDIS2_0_EVENTLOOP_H
