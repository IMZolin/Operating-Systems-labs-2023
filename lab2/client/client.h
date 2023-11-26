#pragma once

#include <semaphore.h>
#include "connections/conn.h"

class Client
{
private:
    bool isRun = false;
    pid_t hostPid = 0;
    sem_t *hostSemaphore;
    sem_t *clientSemaphore;
    std::unique_ptr<Connection> conn;

    Client();
    Client(Client &c) = delete;
    Client &operator=(const Client &c) = delete;
    static void signalHandler(int signal);
    bool getHostMessage(Message &msg);
    size_t getClientNumber(CLIENT_STATE clientState);
    bool sendClientMessage(const Message &msg);
    bool stopClient();

public:
    static Client &getInstance();
    ~Client();
    bool init(const pid_t &hostPid);
    void run();
};