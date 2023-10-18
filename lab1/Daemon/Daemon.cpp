#include "Daemon.hpp"

void Daemon::Run(){
    syslog(LOG_NOTICE, "Daemon started");
    while(isRunning){
        syslog(LOG_NOTICE, "Daemon running");
    }
}

void Daemon::Terminate(){
    syslog(LOG_NOTICE, "Process terminated");
    isRunning = false;
    closelog();
}

void Daemon::SetConfigPath(const std::string &config_path){
    
    if (std::filesystem::exists(config_path)){
        configPath = std::filesystem::absolute(config_path);
    }
    else{
        throw std::runtime_error("File does not exist");
    }     
}

void Daemon::LoadConfig(const std::string &config_file){
    SetConfigPath(config_file);
    entries.clear();
    std::ifstream f(configPath);
        std::string dir1, dir2, time;
        while (f >> dir1 >> dir2 >> time) {
            entries.push_back(std::make_tuple(std::filesystem::path(dir1), std::filesystem::path(dir2), std::time_t(std::stoi(time))));
        }
}