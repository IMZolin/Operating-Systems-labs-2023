#include <cstring>
#include <iostream>

#include "hostHandler.h"

int main( int argc, char **argv ) {
    Host &host = Host::getInstance();
    if (host.init()) {
        host.run();
    } else {
        std::cout << "ERROR: errno = " << strerror(errno) << std::endl;
        return 1;
    }

    return 0;
}