#include <iostream>
#include "Daemon/Daemon.hpp"

int main(int argc, char *argv[]) {
    std::string config_file;
    if (argc == 1){
        config_file = "config.txt";
    }
    else if (argc == 2){
        config_file = argv[1];
    }   
    else {
        std::cout << "Uncorrent count of arguments" << std::endl;
        return 1;
    }
    Daemon& daemon = Daemon::getInstance();
    daemon.LoadConfig("config.txt");
    return 0;
}