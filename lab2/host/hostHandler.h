#pragma once

#include <semaphore.h>
#include <memory>
#include "../utils/configuration.h"
#include "../conn/conn.h"

class Host {
private:
    std::unique_ptr<Connection> conn;
    sem_t *hostSemaphore;
    sem_t *clientSemaphore;
    int wolfNum;

    Host();
    Host(const Host &host) = delete;
    Host &operator=(const Host &host) = delete;
    void getHostNum();
    void process();
    void close();

public:
    pid_t pid;
    static Host &getInstance();
    bool init();
    void run();
};
