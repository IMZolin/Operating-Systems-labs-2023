#pragma once

#include <sys/socket.h>
#include <string>
#include "conn.h"

class Socket : public Connection
{
private:
    const std::string SOCKET_ROUTE = "/tmp/socket_conn_";
    const size_t MAX_CLIENT_NUM = 1;
    socklen_t hostSocket;
    socklen_t clientSocket;
    bool isHost = false;
    std::string name;

public:
    Socket() = default;
    ~Socket() = default;
    bool open(int pid, bool isHost) final;
    bool read(void *buf, size_t count) const final;
    bool write(void *buf, size_t count) final;
    bool close() final;
};