#pragma once

#include <unistd.h>
#include <semaphore.h>
#include <csignal>

#include "connections/conn.h"
#include <atomic>
#include <thread>
#include <functional>

class Host{
    public:
        static Host &getInstance();
        ~Host();

        void run();

        static bool isRunning();
        static void stopRunning();
        static void getNewHostMessage(unsigned short thrownNum);
    private:
        Host();
        Host(Host &host) = delete;
        Host &operator=(const Host &host) = delete;
        bool stop();
        bool getClientMessage(Message &msg);
        bool sendHostMessage(const Message &msg);
        CLIENT_STATE updateClientState(const size_t &hostNum, const Message& clientMessage);
        bool checkRun(CLIENT_STATE clientState);
        static void signalHandle(int sig, siginfo_t *sigInfo, void *ptr);

        bool isRun = false;
        sem_t *semHost{};
        sem_t *semClient{};
        pid_t hostPid = 0;
        pid_t clientPid = 0;
        std::unique_ptr<Connection> conn;

        std::atomic<Message> hostMessage;

        bool isNewMessage = false;

};