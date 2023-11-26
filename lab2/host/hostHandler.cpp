#include <sys/syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include "hostHandler.h"
#include "utils/configuration.h"
#include "client/client.h"

Host &Host::getInstance()
{
    static Host instance;
    return instance;
}

Host::Host()
{
    conn = Connection::create();
    hostPid = getpid();
    openlog("HostAndClient", LOG_NDELAY | LOG_PID, LOG_USER);
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
        syslog(LOG_ERR, "ERROR: Can't close connection");
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

void Host::signalHandle(int sig, siginfo_t *sigInfo, void *ptr)
{
    switch (sig)
    {
    case SIGTERM:
        Host::getInstance().isRun = false;
        return;
    case SIGINT:
        syslog(LOG_INFO, "INFO: host terminate");
        exit(EXIT_SUCCESS);
        return;
    default:
        syslog(LOG_INFO, "INFO: unknown command");
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

bool Host::checkRun(CLIENT_STATE clientState)
{
    if (clientState == CLIENT_STATE::ALIVE || clientState == CLIENT_STATE::ALMOST_DEAD)
    {
        return true;
    }
    syslog(LOG_INFO, "INFO: goat dead. Game over");
    return false;
}

bool Host::stop()
{
    if (sem_post(semClient) == -1)
    {
        syslog(LOG_ERR, "ERROR: Can't continue client");
        return false;
    }
    if (sem_wait(semHost) == -1)
    {
        syslog(LOG_ERR, "ERROR: Can't stop host");
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




