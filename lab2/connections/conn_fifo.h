#pragma once

#include <string>
#include "conn.h"

class Fifo : public Connection
{
private:
    const std::string FIFO_ROUTE = "/tmp/fifo_conn_";
    bool isHost = false;
    std::string name;
    int descriptor = -1;

public:
    Fifo() = default;
    ~Fifo() = default;
    bool open(pid_t pid, bool isHost) final;
    bool read(Message &msg) const final;
    bool write(const Message &msg) final;
    bool close() final;
};