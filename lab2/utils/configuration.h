#pragma once

#include <cstdlib>
#include <string>

namespace Configuration
{
    static const std::string HOST_SEMAPHORE_NAME = "host_semaphore";
    static const std::string CLIENT_SEMAPHORE_NAME = "client_semaphore";

    static const size_t TIME_OUT = 5;
    static const size_t ROUND_TIME_IN_MS = 3000;

    namespace Host  
    {
        static const size_t MIN_NUMBER = 1;
        static const size_t MAX_NUMBER = 100;
    };

    namespace Client
    {
        static const size_t MIN_NUMBER = 1;
        static const size_t MAX_ALIVE_NUMBER = 100;
        static const size_t MAX_ALMOST_DEATH_NUMBER = 50;
        static const size_t ALMOST_DEAD_CLIENT_VALIDATION = 20;
        static const size_t ALIVE_CLIENT_VALIDATION = 70;
    };
};