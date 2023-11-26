#include <sys/syslog.h>
#include <iostream>
#include <sys/un.h>
#include "conn_mq.h"

std::unique_ptr<Connection> Connection::create()
{
    return std::make_unique<MessageQueue>();
}

bool MessageQueue::open(pid_t pid, bool isHost)
{
    this->isHost = isHost;
    std::string name = std::string(MQ_ROUTE + std::to_string(pid));
    if (isHost)
    {
        struct mq_attr attr;
        attr.mq_maxmsg = MAX_QUEUE_SIZE;
        attr.mq_msgsize = sizeof(Message);
        attr.mq_curmsgs = 0;
        attr.mq_flags = 0;
        descriptor = mq_open(name.c_str(), O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO, &attr);
    }
    else
    {
        descriptor = mq_open(name.c_str(), O_RDWR);
    }

    if (descriptor < 0)
    {
        syslog(LOG_ERR, "ERROR: Can't open message queue");
        return false;
    }

    return true;
}

bool MessageQueue::read(Message &msg) const
{
    if (mq_receive(descriptor, (char *)(&msg), sizeof(Message), nullptr) < 0)
    {
        syslog(LOG_ERR, "ERROR: Can't read message");
        return false;
    }
    return true;
}

bool MessageQueue::write(const Message &msg)
{
    if (mq_send(descriptor, (char *)(&msg), sizeof(Message), 0) < 0)
    {
        syslog(LOG_ERR, "ERROR: Can't write message");
        return false;
    }
    return true;
}

bool MessageQueue::close()
{
    if (isHost && mq_close(descriptor) < 0)
    {
        syslog(LOG_ERR, "ERROR: Can't close connection");
        return false;
    }
    return true;
}