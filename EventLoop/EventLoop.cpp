#include <cstdio>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <iostream>
#include <vector>
#include <map>
#include "EventLoop.h"
#include "../Networking/TCP/Errors/TCPServerException.h"

//HashTable PlaceHolder
// TODO: DELETE ME
static std::map<std::string, std::string> g_map;


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
    int32_t err = handle_request(&conn->r_buf[4], len, &resCode, &conn->w_buf[4 + 4], &wLen);
    if (err) {
        conn->state = STATE_END;
        return false;
    }

    wLen += 4;
    memcpy(&conn->w_buf[0], &wLen, 4);
    memcpy(&conn->w_buf[4], &resCode, 4);
    conn->w_buf_size = 4 + wLen;

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

int32_t EventLoop::handle_request(const uint8_t *req, uint32_t reqLen, uint32_t *resCode, uint8_t *res, uint32_t *resLen) {
    std::vector<std::string> cmd;
    if (0 != parse_request(req, reqLen, cmd)) {
        return -1;
    }

    if (cmd.size() == 2 ) {
        if ((strcasecmp((const char *) cmd[0].data(), "get")) == 0) {
            // Handle Get Commands
            *resCode = handle_get_request(cmd, res, resLen);
        } else if (strcasecmp(cmd[0].data(), "del") == 0) {
            //Handle Delete Commands
        }   *resCode = handle_del_request(cmd, res, resLen);
    } else if (cmd.size() == 3) {
        if (strcasecmp(cmd[0].data(), "set") == 0) {
            // Handle Set Commands
            *resCode = handle_set_request(cmd, res, resLen);
        }
    } else {
        *resCode = RES_ERR;
        const char* msg = "Unknown Command";
        strcpy((char *)res, msg);

        *resLen = strlen(msg);
        return 0;
    }

    return 0;
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

uint32_t EventLoop::handle_get_request(const std::vector<std::string> &cmd, uint8_t *res, uint32_t *resLen) {
    if (!g_map.count(cmd[1])) {
        return RES_NX;
    }

    std::string &val = g_map[cmd[1]];
    assert(val.size() <= k_max_msg);

    memcpy(res, val.data(), val.size());
    *resLen = (uint32_t)val.size();
    return RES_OK;
}

uint32_t EventLoop::handle_set_request(const std::vector<std::string> &cmd, uint8_t *res, uint32_t *resLen) {
    (void) res;
    (void) resLen;

    g_map[cmd[1]] = cmd[2];

    return RES_OK;
}

uint32_t EventLoop::handle_del_request(const std::vector<std::string> &cmd, uint8_t *res, uint32_t *resLen) {
    (void) res;
    (void) resLen;

    g_map.erase(cmd[1]);
    return RES_OK;
}
