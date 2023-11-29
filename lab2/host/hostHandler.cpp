#include <csignal>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <cmath>
#include <fcntl.h>
#include <cstring>
#include <ctime>
#include <random>
#include<string>

#include "hostHandler.h"
#include "../client/client.h"

Host::Host() : wolfNum(0)
{
    openlog("WolfAndGoat", LOG_NDELAY | LOG_PID, LOG_USER);
    conn = Connection::create();
}

Host &Host::getInstance() {
    static Host instance;
    return instance;
}

void handler(int signum) {
    static Host &instance = Host::getInstance();
    if (instance.pid > 0)
        kill(instance.pid, signum);
    exit(signum);
}

bool Host::init() {
    syslog(LOG_INFO, "[INFO]: Host initialization");
    sem_unlink(Configuration::HOST_SEMAPHORE_NAME.c_str());
    sem_unlink(Configuration::CLIENT_SEMAPHORE_NAME.c_str());

    hostSemaphore = sem_open(Configuration::HOST_SEMAPHORE_NAME.c_str(), O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (hostSemaphore == SEM_FAILED) {
        std::cout << "ERROR: Can't open host semaphore, errno = " << strerror(errno);
        syslog(LOG_ERR, "[ERROR]: Host semaphore not created");
        return false;
    }
    clientSemaphore = sem_open(Configuration::CLIENT_SEMAPHORE_NAME.c_str(), O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (clientSemaphore == SEM_FAILED) {
        sem_unlink(Configuration::HOST_SEMAPHORE_NAME.c_str());
        std::cout << "ERROR: Can't open client semaphore, errno = " << strerror(errno);
        syslog(LOG_ERR, "[ERROR]: Client semaphore not created");
        return false;
    }

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGKILL);
    sigaddset(&set, SIGSTOP);
    act.sa_mask = set;
    sigaction(SIGTERM, &act, 0);
    sigaction(SIGKILL, &act, 0);
    sigaction(SIGSTOP, &act, 0);
    syslog(LOG_INFO, "[INFO]: Host initialize successfully");
    return true;
}

void Host::run() {
    switch (pid = fork()) {
        case -1:
            syslog(LOG_INFO, "[ERROR]: Can't create client process");
        case 0: {
            Client &client = Client::getInstance();
            if (client.init(hostSemaphore, clientSemaphore)) {
                client.run();
            }
            break;
        }
        default: {
            std::cout << "Successfully created client process" << std::endl;
            if (conn->open(0, true))
            {
                sem_post(clientSemaphore);
                process();
                close();
            }
            break;
        }
    }
}

void Host::process() {
    Message msg;
    const int TIMEOUT_SEC = 5;

    int round = 0;

    while (true) {
        struct timespec ts;
        timespec_get(&ts, TIMER_ABSTIME);
        ts.tv_sec += TIMEOUT_SEC;
        if (sem_timedwait(hostSemaphore, &ts) == -1)
        {
            std::cout << "Timeout" << std::endl;
            msg.num = -1;
            sem_post(clientSemaphore);
            break;
        }
        getHostNum();
        if (conn->read(&msg, sizeof(msg))) {
            std::cout << "Goat number: " << msg.num << std::endl;
            if (msg.state == 0) {
                if (std::abs(wolfNum - msg.num) > static_cast<int>(Configuration::Client::ALIVE_CLIENT_VALIDATION))
                    msg.state = DEAD;
            } else {
                if (std::abs(wolfNum - msg.num) <= static_cast<int>(Configuration::Client::ALMOST_DEAD_CLIENT_VALIDATION))
                    msg.state = ALIVE;
                else {
                    syslog(LOG_INFO, "[INFO]: Game over!");
                    std::cout << "Game over!" << std::endl;
                    msg.num = -1;
                    conn->write(&msg, sizeof(msg));
                    sem_post(clientSemaphore);
                    break;
                }
            }
            std::cout << "Round" + round << std::endl;
            if (msg.state == ALIVE)
                std::cout << "Goat is alive" << std::endl;
            else
                std::cout << "Goat is dead" << std::endl;
            conn->write(&msg, sizeof(msg));
            round++;
        }
        sem_post(clientSemaphore);
    }
}

void Host::getHostNum() {
    std::cout << "Input wolf number:" << std::endl;
    std::string word;
    int num = 0;
    do {
        fd_set fds;
        int console = fileno(stdin);
        FD_ZERO(&fds);
        FD_SET(console, &fds);
        struct timeval timeout;
        timeout.tv_sec = 3; 
        timeout.tv_usec = 0;
        int ready = select(console + 1, &fds, nullptr, nullptr, &timeout);
        if (ready == -1) {
            std::cerr << "Error in select()\n";
        }
        else if (ready == 0) {
            std::random_device rd;
            std::mt19937 mt(rd());
            std::uniform_int_distribution<int> dist(Configuration::Host::MIN_NUMBER, Configuration::Host::MAX_NUMBER);
            num = dist(mt);
            std::cout << num << std::endl;
        } 
        else {
            if (FD_ISSET(console, &fds)) {
                char buffer[256];
                read(console, buffer, sizeof(buffer));
                try {
                    num = std::atoi(buffer);
                } catch (std::exception &e) {
                    std::cout << "Try again" << std::endl;
                }
                if (num < static_cast<int>(Configuration::Host::MIN_NUMBER) || num > static_cast<int>(Configuration::Host::MAX_NUMBER)) {
                    std::cout << "Number must be between 1 and 100" << std::endl;
                }
            }
        }
    } while (num < static_cast<int>(Configuration::Host::MIN_NUMBER) || num > static_cast<int>(Configuration::Host::MAX_NUMBER));
    wolfNum = num;
}

void Host::close()
{
  conn->close();

  if (sem_close(hostSemaphore) == -1)
    std::cout << "ERROR: Can't close host semaphore, errno = " << strerror(errno) << std::endl;
  if (sem_unlink(Configuration::HOST_SEMAPHORE_NAME.c_str()) == -1)
    std::cout << "ERROR: Can't unlink host semaphore, errno = " << strerror(errno) << std::endl;
  if (sem_close(clientSemaphore) == -1)
    std::cout << "ERROR: Can't close client semaphore, errno = " << strerror(errno) << std::endl;
  if (sem_unlink(Configuration::CLIENT_SEMAPHORE_NAME.c_str()) == -1)
    std::cout << "ERROR: Can't unlink client semaphore, errno = " << strerror(errno) << std::endl;
}
