#include <sys/stat.h>
#include <sys/syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <iostream>

#include "conn_fifo.h"

std::unique_ptr<Connection> Connection::create()
{
    return std::make_unique<Fifo>();
}

bool Fifo::open(int pid, bool isHost)
{
    name = std::string(FIFO_ROUTE + std::to_string(pid));
    this->isHost = isHost;
    if (isHost)
    {
        if (mkfifo(name.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) < 0)
        {
            syslog(LOG_ERR, "[ERROR]: Can't create fifo file");
            return false;
        }
    }
    descriptor = ::open(name.c_str(), O_RDWR);
    if (descriptor < 0)
    {
        syslog(LOG_ERR, "[ERROR]: Can't open fifo file");
        return false;
    }

    return true;
}

bool Fifo::read(void *buf, size_t count) const
{
    if (::read(descriptor, buf, count) < 0)
    {
        syslog(LOG_ERR, "[ERROR]: Can't read message");
        return false;
    }
    return true;
}

bool Fifo::write(void *buf, size_t count)
{
    Message *msg = (Message *) buf;
    if (::write(descriptor, msg, count) < 0)
    {
        syslog(LOG_ERR, "[ERROR]: Can't write message");
        return false;
    }
    return true;
}

bool Fifo::close()
{
    if (::close(descriptor) < 0)
    {
        syslog(LOG_ERR, "[ERROR]: Can't close file");
        return false;
    }
    if (isHost)
    {
        if (unlink(name.c_str()) < 0)
        {
            syslog(LOG_ERR, "[ERROR]: Can't unlink file");
            return false;
        }
    }
    return true;
}