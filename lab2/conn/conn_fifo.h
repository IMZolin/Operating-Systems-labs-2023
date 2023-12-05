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
    bool open(int pid, bool isHost) final;
    bool read(void *buf, size_t count) const final;
    bool write(void *buf, size_t count) final;
    bool close() final;
};