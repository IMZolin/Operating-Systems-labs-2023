#pragma once
#include <string>
#include <semaphore.h>
#include <sys/syslog.h>
#include <csignal>

#include "../conn/conn.h"
#include "../utils/configuration.h"

class Client {
private:
    std::unique_ptr<Connection> conn;
    sem_t *hostSemaphore;
    sem_t *clientSemaphore;
    Client();
    Client(Client &c) = delete;
    Client &operator=(const Client &c) = delete;

public:
    static Client &getInstance();
    ~Client();
    bool init(sem_t *hosSem, sem_t *clientSem);
    void run();
};
