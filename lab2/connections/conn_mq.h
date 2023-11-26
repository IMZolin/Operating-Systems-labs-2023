#pragma once

#include <string>
#include <mqueue.h>
#include "conn.h"

class MessageQueue : public Connection
{
private:
    const std::string MQ_ROUTE = "/mq_conn_";
    const size_t MAX_QUEUE_SIZE = 1;
    bool isHost = false;
    mqd_t descriptor;

public:
    MessageQueue() = default;
    ~MessageQueue() = default;

    bool open(pid_t pid, bool isHost) final;
    bool read(Message &msg) const final;
    bool write(const Message &msg) final;
    bool close() final;
};