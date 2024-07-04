#include <cstdio>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <iostream>
#include <vector>
#include "absl/container/flat_hash_map.h"
#include "EventLoop.h"
#include "../Networking/TCP/Errors/TCPServerException.h"
#include "../Common/LikeRedis/LikeRedisCommonHelpers.h"

static absl::flat_hash_map<std::string, std::string> g_map;

EventLoop::EventLoop(int serverFd) {
    std::vector<epoll_event> events;
    std::vector<Conn *> connections = std::vector<Conn*>{};

    struct epoll_event event{};
    this->epollFd = epoll_create1(0);
    if (this->epollFd == -1) {
        perror("[-] Error creating new epoll instance");
        exit(-1);
    }
    // Include Server Socket For Listening Connections
    event.events = EPOLLIN;
    event.data.fd = serverFd;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, serverFd, &event) == -1) {
        perror("[-] Error adding server socket to epoll instance");
        exit(-1);
    }
    events.push_back(event);
    while (true) {
        try {
            // Check if There Are Connections in Queue
            if (!connections.empty()) {
                for (Conn*  connection : connections) {
                    if (!connection) {
                        continue;
                    }

                    event.data.fd = connection->fd;
                    event.events = (connection->state == STATE_REQ) ? EPOLLIN : EPOLLOUT;
                    event.events |= EPOLLERR;

                    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, connection->fd, &event) == -1) {
                        perror("[-] Error adding client connection socket to epoll instance");
                        exit(-1);
                    }
                }
            }
            errno = 0;

            int numEvents = epoll_wait(this->epollFd, events.data(), this->maxEvents, -1);
            if (numEvents == -1) {
                switch (errno) {
                    case EBADF:
                        fprintf(stderr, "[-] Bad descriptor (epoll_wait)\n");
                        break;
                    case EFAULT:
                        fprintf(stderr, "[-] Events Memory Address Not Accessible (epoll_wait)\n");
                        break;
                    case EINVAL:
                        fprintf(stderr, "[-] epoll fd is not a valid descriptor (epoll_wait)\n");
                        break;
                    default:
                        perror("[-] error waiting on the epoll instance");
                }
                exit(-1);
            }

            for (int i = 0; i < numEvents; i++) {
                if (events[i].data.fd == serverFd) {
                    // Handle incoming connection
                    accept_new_connection(serverFd, connections);
                } else {
                    Conn* conn = connections[events[i].data.fd];

                    // Handle Connection I/O
                    connection_io(conn);

                    // Check For Connections That Need To Be Closed
                    if (conn->state == STATE_END) {
                        // Client Close Connection Get Rid Of It
                        connections[events[i].data.fd] = nullptr;
                        close(conn->fd);
                        free(conn);
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void EventLoop::connection_io(Conn *conn) {
    if (conn->state == STATE_REQ) {
        state_req(conn);
    } else if (conn->state == STATE_RES) {
        state_res(conn);
    } else {
        assert(0);
    }
}

void EventLoop::state_req(Conn *conn) {
    while(try_fill_buffer(conn)) {}
}

bool EventLoop::try_fill_buffer(Conn *conn) {
    assert(conn->r_buf_size < sizeof(conn->r_buf));
    ssize_t rv;
    do {
        size_t cap = sizeof(conn->r_buf) - conn->r_buf_size;
        rv = read(conn->fd, &conn->r_buf[conn->r_buf_size], cap);
    } while (rv < 0 && errno == EINTR);

    if (rv < 0 && errno == EAGAIN){
        return false;
    }

    if (rv < 0) {
        perror("[-] Error reading from client socket");
        conn->state = STATE_END;
        return false;
    }

    if (rv == 0) {
        if (conn->r_buf_size > 0) {
            printf("[-] Unexpected EOF\b");
        } else {
            printf("[-] EOF\b");
        }
        conn->state = STATE_END;
        return false;
    }

    conn->r_buf_size += rv;
    assert((conn->r_buf_size <= sizeof(conn->r_buf)) - conn->r_buf_size);

    // Process Requests One By One
    while(try_one_request(conn)) {}

    return (conn->state == STATE_REQ);
}

bool EventLoop::try_one_request(Conn *conn) {
    if (conn->r_buf_size == 4) {
        // Not Enough Data In Buffer Yet
        return false;
    }

    uint32_t len = 0;
    memcpy(&len, conn->r_buf, 4);
    if (len > k_max_msg) {
        printf("[-] message too long\n");
        conn->state = STATE_END;
        return false;
    }

    if (4 + len > conn->r_buf_size) {
        // Not Enough Data Yet
        return false;
    }

    uint32_t resCode = 0;
    uint32_t wLen = 0;

    LR_RESPONSE lr{};
    auto request = (LR_REQUEST *)&conn->r_buf[4];

    handle_request(request, lr);
    if (4 + lr.dataLen > k_max_msg) {
        err_response(lr, 2000, "Response too big");
        return false;
    }

    memcpy(&conn->w_buf, &lr, sizeof(lr));
    conn->w_buf_size = sizeof(lr);

    size_t remain = conn->r_buf_size - 4 - len;
    if (remain) {
        memmove(conn->r_buf, &conn->r_buf[4 + len], remain);
    }

    conn->r_buf_size = remain;
    conn->state = STATE_RES;

    state_res(conn);
    return (conn->state == STATE_REQ);
}

void EventLoop::state_res(Conn *conn) {
    while (try_flush_buffer(conn)) {}
}

bool EventLoop::try_flush_buffer(Conn *conn) {
    ssize_t rv;

    do {
        size_t remain = conn->w_buf_size - conn->w_buf_sent;
        rv = write(conn->fd, &conn->w_buf[conn->w_buf_sent], remain);
    } while (rv < 0 && errno == EINTR);

    if (rv < 0 && errno == EAGAIN) {
        return false;
    }

    if (rv < 0) {
        perror("[-] write error");
        conn->state = STATE_END;
        return false;
    }

    conn->w_buf_sent += rv;
    assert(conn->w_buf_sent <= conn->w_buf_size);

    if (conn->w_buf_sent == conn->w_buf_size) {
        // Finished Sending Back Data
        conn->state = STATE_REQ;
        conn->w_buf_sent = 0;
        conn->w_buf_size = 0;
        return false;
    }

    return true;
}

void EventLoop::connection_put(Conn* conn, std::vector<Conn*>& connections) {
    if (connections.size() <= (size_t) conn->fd) {
        connections.resize(conn->fd + 1);
    }
    connections[conn->fd] = conn;
}

int32_t EventLoop::accept_new_connection(int fd, std::vector<Conn*>& connections) {
    struct sockaddr_in client_address{};
    socklen_t socklen = sizeof(client_address);

    int connFd = accept(fd, (struct sockaddr*)&client_address, &socklen);
    if (connFd < 0) {
        perror("Error Accepting Client");
        exit(-1);
    }

    auto* conn = (struct Conn*)malloc(sizeof(struct Conn));
    if (!conn) {
        perror("[-] Error allocating memory for connection");
        exit(-1);
    }

    conn->fd = connFd;
    conn->state = STATE_REQ;
    conn->r_buf_size = 0;
    conn->w_buf_size = 0;
    conn->w_buf_sent = 0;

    connection_put(conn, connections);
    return 0;
}

void EventLoop::handle_request(LR_REQUEST* req, LR_RESPONSE &response) {
    std::vector<std::string> cmd;
    switch (req->command) {
        case GET:
            handle_get_request(req, response);
            break;
        case DEL:
            handle_del_request(req, response);
            break;
        case SET:
            handle_set_request(req, response);
            break;
        case KEYS:
            handle_keys_request(req, response);
            break;
        default:
            err_response(response, 2204, "Unknown Command");
            break;
    }
}

int32_t EventLoop::parse_request(const uint8_t *data, size_t len, std::vector<std::string> &out) {
    if (len  < 4) {
        return -1;
    }

    uint32_t k_max_args = 2;
    uint32_t n = 0;
    memcpy(&n, &data[0], 4);
    if (n < k_max_args) {
        return -1;
    }

    size_t pos = 4;
    while (n--) {
        if (pos + 4  > len) {
            return -1;
        }

        uint32_t sz = 0;
        memcpy(&sz, &data[pos], 4);
        if (pos + 4 + sz > len) {
            return -1;
        }

        out.emplace_back((char *)&data[pos + 4], sz);

        pos += 4 + sz;
    }


    if (pos != len) {
        return -1;
    }
    return 0;
}

void EventLoop::handle_get_request(LR_REQUEST *request, LR_RESPONSE &response) {
    if (!g_map.count(request->key)) {
        nil_response(response);
        return;
    }

    auto val = g_map.find(std::string(request->key));
    assert(val->second.size() <= k_max_msg);

    str_response(response, val->second);
}

void EventLoop::handle_set_request(LR_REQUEST *request, LR_RESPONSE &response) {
    char dataStr[request->dataLen + 1];
    memset(dataStr, 0, request->dataLen + 1);
    memcpy(dataStr, request->data, request->dataLen);

    g_map.try_emplace(request->key, std::string(dataStr));
    nil_response(response);
}

void EventLoop::handle_del_request(LR_REQUEST *request, LR_RESPONSE &response) {
    g_map.erase(request->key);
    nil_response(response);
}


void EventLoop::handle_keys_request(LR_REQUEST *request, LR_RESPONSE &response) {
    arr_response(response, g_map.size());
    for (const auto &[key, value] : g_map) {
        str_response(response, key);
    }
}

void EventLoop::nil_response(LR_RESPONSE &response) {
    response.serialization = SER_NIL;
}

void EventLoop::str_response(LR_RESPONSE &response, const std::string &val) {
    response.serialization = SER_STR;
    response.dataLen = val.size();

    response.data = calloc(1, response.dataLen);
    if (!response.data) {
        throw TCPServerException("Error allocating memory for response data (nil_response)");
    }

    memcpy(response.data, val.data(), response.dataLen);
}

void EventLoop::int_response(LR_RESPONSE &response, int64_t val) {
    struct LR_RESPONSE lr{};
    lr.statusCode = 200;
    lr.dataLen = sizeof(val);

    lr.data = calloc(1, sizeof(val));
    if (lr.data) {
        throw TCPServerException("Error allocating memory for response buffer");
    }
}

void EventLoop::err_response(LR_RESPONSE &response, int32_t code, const std::string &msg) {
    response.serialization = SER_ERR;
    response.statusCode = code;
    response.dataLen = msg.size();

    memcpy(response.data, msg.data(), msg.size());
}

void EventLoop::arr_response(LR_RESPONSE &response, uint32_t n) {
    response.serialization = SER_ARR;
    response.dataLen = n;
}

