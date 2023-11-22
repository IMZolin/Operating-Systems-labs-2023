#pragma once

#include <unistd.h>
#include <semaphore.h>
#include <csignal>

// #include "connections/conn.h"
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
        static void getNewWolfMessage(unsigned short thrownNum);
    private:
        Host();
        Host(Host &host) = delete;
        Host &operator=(const Host &host) = delete;
        bool stop();
        bool getGoatMessage(Message &msg);
        bool sendWolfMessage(const Message &msg);
        GOAT_STATE updateGoatState(const size_t &wolfNum, const Message& goatMessage);
        bool checkRun(GOAT_STATE goatState);
        static void signalHandle(int sig, siginfo_t *sigInfo, void *ptr);
}