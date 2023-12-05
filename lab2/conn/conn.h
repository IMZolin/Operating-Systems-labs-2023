#pragma once

#include "../utils/message.h"
#include "sys/types.h"
#include <memory>
#include <cstdlib>
#include <string>

class Connection {
public:
    static std::unique_ptr<Connection> create();

    virtual ~Connection() = default;
    virtual bool open(int pid, bool isHost) = 0;
    virtual bool read(void *buf, size_t count) const = 0;
    virtual bool write(void *buf, size_t count) = 0;
    virtual bool close() = 0;
};
