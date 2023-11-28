#include "hostHandler.h"


#include <iostream>
#include <sys/syslog.h>

int main()
{
    if (Host::getInstance().init())
    {
        std::thread hostThread(&Host::run, &Host::getInstance());
        hostThread.join();
    }
    else
    {
        std::cout << "ERROR: Can't initialize host" << std::endl;
        syslog(LOG_ERR, "[ERROR]: Can't initialize host");
    }
    syslog(LOG_INFO, "[INFO]: Game over!");
    return 0;
}