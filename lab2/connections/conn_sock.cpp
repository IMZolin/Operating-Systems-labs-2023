#include <sys/un.h>
#include <sys/syslog.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include "conn_sock.h"
#include "utils/configuration.h"

std::unique_ptr<Connection> Connection::create()
{
    return std::make_unique<Socket>();
}

bool Socket::open(pid_t pid, bool isHost)
{
    struct sockaddr_un addr;
    this->isHost = isHost;
    name = SOCKET_ROUTE + std::to_string(pid);
    addr.sun_family = AF_UNIX;

    std::strncpy(addr.sun_path, name.c_str(), sizeof(addr.sun_path) - 1);

    if (isHost)
    {
        if ((hostSocket = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0)
        {
            syslog(LOG_ERR, "ERROR: Can't create host socket");
            return false;
        }

        if (bind(hostSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            syslog(LOG_ERR, "ERROR: Can't bind host socket");
            return false;
        }

        if ((listen(hostSocket, MAX_CLIENT_NUM)) < 0)
        {
            syslog(LOG_ERR, "ERROR: Can't listen connection");
            return false;
        }

        if ((clientSocket = accept(hostSocket, NULL, NULL)) < 0)
        {
            syslog(LOG_ERR, "ERROR: Can't accept client socket");
            return false;
        }
    }
    else
    {
        if ((clientSocket = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0)
        {
            syslog(LOG_ERR, "ERROR: Can't create client socket");
            return false;
        }
        if (connect(clientSocket, (const struct sockaddr *)&addr, sizeof(sockaddr_un)) < 0)
        {
            std::cout << strerror(errno) << std::endl;
            syslog(LOG_ERR, "ERROR: Can't connect sockets");
            return false;
        }
    }

    return true;
}

bool Socket::read(Message &msg) const
{
    if (recv(clientSocket, &msg, sizeof(Message), 0) < 0)
    {
        syslog(LOG_ERR, "ERROR: Can't read from socket");
        return false;
    }

    return true;
}

bool Socket::write(const Message &msg)
{
    if (send(clientSocket, &msg, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        syslog(LOG_ERR, "ERROR: Can't write to socket");
        return false;
    }

    return true;
}

bool Socket::close()
{
    if (isHost)
    {
        if (::close(clientSocket) < 0)
        {
            syslog(LOG_ERR, "ERROR: Can't close client socket");
            return false;
        }

        if (::close(hostSocket) < 0)
        {
            syslog(LOG_ERR, "ERROR: Can't close host socket");
            return false;
        }

        if (unlink(name.c_str()) < 0)
        {
            syslog(LOG_ERR, "ERROR: Can't unlink socket");
            return false;
        }
    }
    else
    {
        if (::close(clientSocket) < 0)
        {
            syslog(LOG_ERR, "ERROR: Can't close client socket");
            return false;
        }
    }

    return true;
}