#pragma once

#include <unistd.h>
#include <semaphore.h>
#include <csignal>
#include <atomic>
#include <thread>
#include <functional>
#include "connections/conn.h"

class Host{
    private:
        bool isRun = false;
        sem_t *semHost{};
        sem_t *semClient{};
        pid_t hostPid = 0;
        pid_t clientPid = 0;
        std::unique_ptr<Connection> conn;
        std::atomic<Message> hostMessage;
        bool isNewMessage = false;

        Host();
        Host(Host &host) = delete;
        Host &operator=(const Host &host) = delete;
        bool stop();
        bool getClientMessage(Message &msg);
        bool sendHostMessage(const Message &msg);
        void getHostNum();
        CLIENT_STATE updateClientState(const size_t &hostNum, const Message& clientMessage);
        bool checkRun(CLIENT_STATE clientState);
        static void signalHandle(int sig, siginfo_t *sigInfo, void *ptr);
        
    public:
        static Host &getInstance();
        ~Host();
        bool init();
        void run();
        static bool isRunning();
        static void stopRunning();
        static void getNewHostMessage(unsigned short thrownNum);
};