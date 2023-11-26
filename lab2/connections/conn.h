#pragma once

#include "sys/types.h"
#include "utils/message.h"

#include <memory>

class Connection
{
public:
    virtual ~Connection() = default;

    static std::unique_ptr<Connection> create();

    virtual bool open(pid_t pid, bool isHost) = 0;
    virtual bool read(Message &msg) const = 0;
    virtual bool write(const Message &msg) = 0;
    virtual bool close() = 0;
};
