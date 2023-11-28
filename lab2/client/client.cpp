#include <csignal>
#include <sys/syslog.h>
#include <time.h>
#include <random>
#include <iostream>
#include <cstring>
#include "client.h"
#include "utils/configuration.h"

Client &Client::getInstance()
{
    static Client instance;
    return instance;
}

Client::Client()
{
    conn = Connection::create();
    signal(SIGTERM, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGINT, signalHandler);
}

Client::~Client()
{
    if (clientSemaphore != SEM_FAILED)
    {
        sem_close(clientSemaphore);
    }
    if (hostSemaphore != SEM_FAILED)
    {
        sem_close(hostSemaphore);
    }
    if (!conn->close())
    {
        syslog(LOG_ERR, "[ERROR]: Can't close connection");
    }
    kill(hostPid, SIGTERM);
}

bool Client::init(const pid_t &hostPid)
{
    syslog(LOG_INFO, "[INFO]: Client initialization");
    this->hostPid = hostPid;
    if (!conn->open(hostPid, false))
    {
        syslog(LOG_ERR, "[ERROR]: Can't open client connection");
        return false;
    }
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
    isRun = true;
    syslog(LOG_INFO, "[INFO]: Client initialized");
    return true;
}

void Client::signalHandler(int signal)
{
    switch (signal)
    {
    case SIGTERM:
        Client::getInstance().isRun = false;
        exit(EXIT_SUCCESS);
    case SIGINT:
        syslog(LOG_INFO, "[INFO]: Receive delete client request");
        exit(EXIT_SUCCESS);
    case SIGUSR1:
        syslog(LOG_INFO, "[INFO]: Client end game. Game over!");
        kill(Client::getInstance().hostPid, SIGTERM);
        exit(EXIT_SUCCESS);
    default:
        syslog(LOG_INFO, "[INFO]: Unknown command");
    }
}

void Client::run()
{
    syslog(LOG_INFO, "[INFO]: Client start running");
    unsigned short clientNumber = getClientNumber(CLIENT_STATE::ALIVE);
    if (!sendClientMessage({clientNumber, CLIENT_STATE::ALIVE}))
    {
        syslog(LOG_ERR, "[ERROR]: Can't send client message");
        return;
    }
    while (isRun)
    {
        if (!stopClient())
        {
            syslog(LOG_ERR, "[ERROR]: Can't stop client");
            return;
        }
        Message hostMessage;
        if (!getHostMessage(hostMessage))
        {
            syslog(LOG_ERR, "[ERROR]: Can't read host message");
            return;
        }
        clientNumber = getClientNumber(hostMessage.clientState);
        if (!sendClientMessage({clientNumber, hostMessage.clientState}))
        {
            syslog(LOG_ERR, "[ERROR]: Can't send client message");
            return;
        }
        isRun = (hostMessage.clientState != CLIENT_STATE::DEAD);
    }
    syslog(LOG_INFO, "[INFO]: Client run end");
}

bool Client::getHostMessage(Message &msg)
{
    return conn->read(msg);
}

size_t Client::getClientNumber(CLIENT_STATE state)
{
    using namespace Configuration::Client;
    std::random_device seeder;
    std::mt19937 rng(seeder());
    std::uniform_int_distribution<int> genAliveNum(MIN_NUMBER, MAX_ALIVE_NUMBER);
    std::uniform_int_distribution<int> genAlmostDeadNum(MIN_NUMBER, MAX_ALMOST_DEATH_NUMBER);
    size_t res = 0;
    switch (state)
    {
    case CLIENT_STATE::ALIVE:
        res = genAliveNum(rng);
        break;
    case CLIENT_STATE::ALMOST_DEAD:
        res = genAlmostDeadNum(rng);
        break;
    case CLIENT_STATE::DEAD:
        break;
    }
    return res;
}

bool Client::sendClientMessage(const Message &msg)
{
    return conn->write(msg);
}

bool Client::stopClient()
{
    if (sem_post(hostSemaphore) == -1)
    {
        syslog(LOG_ERR, "[ERROR]: Host semaphore can't continue");
        return false;
    }
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += Configuration::TIME_OUT;
    if (sem_timedwait(clientSemaphore, &ts) == -1)
    {
        std::cout << strerror(errno) << std::endl;
        syslog(LOG_ERR, "[ERROR]: Client semaphore can't wait");
        return false;
    }
    return true;
}

