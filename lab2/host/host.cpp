#include "hostHandler.h"


#include <iostream>
#include <sys/syslog.h>

int main()
{
    Host &host = Host::getInstance();
    if (host.init())
    {
        host.run();
    }
    else
    {
        std::cout << "ERROR: Can't initialize host" << std::endl;
        syslog(LOG_ERR, "ERROR: Can't initialize host");
    }
    syslog(LOG_INFO, "INFO: Game over!");
    return 0;
}