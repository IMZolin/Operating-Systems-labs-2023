#include <sys/syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <time.h>
#include <random>
#include "hostHandler.h"
#include "../utils/configuration.h"
#include "../client/client.h"

Host &Host::getInstance()
{
    static Host instance;
    return instance;
}

Host::Host()
{
    conn = Connection::create();
    hostPid = getpid();
    openlog("WolfAndGoat", LOG_NDELAY | LOG_PID, LOG_USER);
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = Host::signalHandle;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGINT, &sig, nullptr);
}

Host::~Host()
{
    kill(clientPid, SIGTERM);
    if (!conn->close())
    {
        syslog(LOG_ERR, "[ERROR]: Can't close connection");
    }
    if (semHost != SEM_FAILED)
    {
        sem_unlink(Configuration::HOST_SEMAPHORE_NAME.c_str());
    }
    if (semClient != SEM_FAILED)
    {
        sem_unlink(Configuration::CLIENT_SEMAPHORE_NAME.c_str());
    }
}

bool Host::isRunning()
{
    return Host::getInstance().isRun;
}

void Host::stopRunning()
{
    Host::getInstance().isRun = false;
}

bool Host::init()
{
    syslog(LOG_INFO, "[INFO]: Host initialization");
    semHost = sem_open(Configuration::HOST_SEMAPHORE_NAME.c_str(), O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (semHost == SEM_FAILED)
    {
        std::cout << "ERROR: Can't open host semaphore, errno = " << strerror(errno);
        syslog(LOG_ERR, "[ERROR]: Host semaphore not created");
        return false;
    }
    semClient = sem_open(Configuration::CLIENT_SEMAPHORE_NAME.c_str(), O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (semClient == SEM_FAILED)
    {
        syslog(LOG_ERR, "[ERROR]: Client semaphore not created");
        sem_unlink(Configuration::HOST_SEMAPHORE_NAME.c_str());
        return false;
    }
    pid_t childPid = fork();
    if (childPid == 0)
    {
        clientPid = getpid();
        if (Client::getInstance().init(hostPid))
        {
            getHostNum();
            Client::getInstance().run();
        }
        else
        {
            syslog(LOG_ERR, "[ERROR]: Client initialization error");
            return false;
        }
        exit(EXIT_SUCCESS);
    }
    if (!conn->open(hostPid, true))
    {
        syslog(LOG_ERR, "[ERROR]: Can't open connection");
        return false;
    }
    Host::getInstance().isRun = true;
    syslog(LOG_INFO, "[INFO]: Host initialize successfully");
    return true;
}

void Host::run()
{
    size_t round = 0;
    while (isRun)
    {
        if (isNewMessage)
        {
            if (!stop())
            {
                syslog(LOG_ERR, "[ERROR]: Can't stop wolf");
                return;
            }
            Message clientMsg;
            if (!getClientMessage(clientMsg))
            {
                syslog(LOG_ERR, "[ERROR]: Can't get message from goat");
                return;
            }
            clientMsg.clientState = updateClientState(hostMessage.load().thrownNumber, clientMsg);
            if (!sendHostMessage({hostMessage.load().thrownNumber, clientMsg.clientState}))
            {
                syslog(LOG_ERR, "[ERROR]: Can't send message to wolf");
                return;
            }
            isRun = checkRun(clientMsg.clientState);
            if (!isRun)
            {
                kill(clientPid, SIGTERM);
                clientPid = 0;
            }
            isNewMessage = false;

            std::cout << "Round" + std::to_string(round) << std::endl;
            std::cout << "Wolf number: " + std::to_string(hostMessage.load().thrownNumber) << std::endl;
            std::cout << "Goat number: " + std::to_string(clientMsg.thrownNumber)<< std::endl;
            round++;
        }
    }
    kill(clientPid, SIGTERM);
}

void Host::getHostNum()
{
    std::cout << "Input wolf number:" << std::endl;
    std::string input;
    size_t res = 0;
    do
    {
        std::getline(std::cin, input);
        try
        {
            res = std::stoi(input);
            if (res < Configuration::Host::MIN_NUMBER || res > Configuration::Host::MAX_NUMBER)
            {
                std::cout << "Number must be between 1 and 100" << std::endl;
            }
        }
        catch (std::invalid_argument const &e)
        {
            std::cout << "Invalid input. Try again." << std::endl;
        }
        catch (std::out_of_range const &e)
        {
            std::cout << "Input out of range. Try again." << std::endl;
        }
    } while (res < Configuration::Host::MIN_NUMBER || res > Configuration::Host::MAX_NUMBER);
    getNewHostMessage(res);
}

void Host::signalHandle(int sig, siginfo_t *sigInfo, void *ptr)
{
    switch (sig)
    {
    case SIGTERM:
        Host::getInstance().isRun = false;
        return;
    case SIGINT:
        syslog(LOG_INFO, "[INFO]: Host terminated");
        exit(EXIT_SUCCESS);
        return;
    default:
        syslog(LOG_INFO, "[INFO]: Unknown command");
    }
}

bool Host::checkRun(CLIENT_STATE clientState)
{
    if (clientState == CLIENT_STATE::ALIVE || clientState == CLIENT_STATE::ALMOST_DEAD)
    {
        return true;
    }
    syslog(LOG_INFO, "[INFO]: Goat dead. Game over!");
    return false;
}

bool Host::stop()
{
    if (sem_post(semClient) == -1)
    {
        syslog(LOG_ERR, "[ERROR]: Can't continue client");
        return false;
    }
    if (sem_wait(semHost) == -1)
    {
        syslog(LOG_ERR, "[ERROR]: Can't stop host");
        return false;
    }
    return true;
}

void Host::getNewHostMessage(unsigned short thrownNum)
{
    Host::getInstance().isNewMessage = true;
    Host::getInstance().hostMessage.store({thrownNum, CLIENT_STATE::ALIVE});
}

bool Host::getClientMessage(Message &msg)
{
    return conn->read(msg);
}

bool Host::sendHostMessage(const Message &msg)
{
    return conn->write(msg);
}

CLIENT_STATE Host::updateClientState(const size_t &hostNum, const Message &clientMessage)
{
    size_t div = std::abs(static_cast<int>(hostNum) - static_cast<int>(clientMessage.thrownNumber));
    switch (clientMessage.clientState)
    {
    case CLIENT_STATE::ALIVE:
        return div > Configuration::Client::ALIVE_CLIENT_VALIDATION ? CLIENT_STATE::ALMOST_DEAD : CLIENT_STATE::ALIVE;
    case CLIENT_STATE::ALMOST_DEAD:
        return div > Configuration::Client::ALMOST_DEAD_CLIENT_VALIDATION ? CLIENT_STATE::DEAD : CLIENT_STATE::ALIVE;
    case CLIENT_STATE::DEAD:
        return CLIENT_STATE::DEAD;
    }
    return CLIENT_STATE::DEAD;
}
