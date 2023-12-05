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

    bool open(int pid, bool isHost) final;
    bool read(void *buf, size_t count) const final;
    bool write(void *buf, size_t count) final;
    bool close() final;
};