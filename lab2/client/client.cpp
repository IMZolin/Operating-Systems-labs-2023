#include <iostream>
#include <random>

#include "client.h"

Client &Client::getInstance() {
    static Client instance;
    return instance;
}

Client::Client() : hostSemaphore(nullptr), clientSemaphore(nullptr)
{
    conn = Connection::create();
}

bool Client::init(sem_t *hostSem, sem_t *clientSem) {
    syslog(LOG_INFO, "[INFO]: Client initialization");
    if (hostSem == nullptr || clientSem == nullptr)
        return false;
    hostSemaphore = hostSem;
    clientSemaphore = clientSem;
    sem_wait(clientSemaphore);
    if ((hostSemaphore = sem_open(Configuration::HOST_SEMAPHORE_NAME.c_str(), 0)) == SEM_FAILED)
    {
        syslog(LOG_ERR, "[ERROR]: Can't connect to host semaphore");
        return false;
    }
    if ((clientSemaphore = sem_open(Configuration::CLIENT_SEMAPHORE_NAME.c_str(), 0)) == SEM_FAILED)
    {
        syslog(LOG_ERR, "[ERROR]: Can't connect to client semaphore");
        return false;
    }
    syslog(LOG_INFO, "[INFO]: Client initialized");
    return conn->open(0, false);
}

void Client::run() {
    syslog(LOG_INFO, "[INFO]: Client start running");
    using namespace Configuration::Client;
    Message msg;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(MIN_NUMBER, MAX_ALIVE_NUMBER);
    msg.num = dist(mt);
    conn->write(&msg, sizeof(msg));
    sem_post(hostSemaphore);

    while (true) {
        sem_wait(clientSemaphore);
        if (!conn->read(&msg, sizeof(msg))) {
            syslog(LOG_ERR, "[ERROR]: Can't read host message");
            sem_post(hostSemaphore);
            return;
        }
        if (msg.num == -1) {
            syslog(LOG_ERR, "[ERROR]: Host semaphore can't continue");
            sem_post(hostSemaphore);
            return;
        }
        std::random_device rd;
        std::mt19937 mt(rd());
        if (msg.state == ALIVE) {
            std::uniform_int_distribution<int> dist(MIN_NUMBER, MAX_ALIVE_NUMBER);
            msg.num = dist(mt);
        } else {
            std::uniform_int_distribution<int> dist(MIN_NUMBER, MAX_ALMOST_DEATH_NUMBER);
            msg.num = dist(mt);
        }
        if (!conn->write(&msg, sizeof(Message))) {
            syslog(LOG_ERR, "[ERROR]: Can't send client message");
            sem_post(hostSemaphore);
            return;
        }
        sem_post(hostSemaphore);
    }
}

Client::~Client() {
    conn->close();
}