#include "Daemon.hpp"

std::string Daemon::configPath;

void Daemon::Run(){
    syslog(LOG_NOTICE, "Daemon started");
    std::vector<std::time_t> last_modified_time(entries.size(), 0);
    isRunning = true;
    while(isRunning){
        syslog(LOG_NOTICE, "Daemon running");
        LoadConfig(configPath);
        const std::vector<std::tuple<std::filesystem::path, std::filesystem::path, std::time_t>> & copy_entries = entries;
        for (size_t idx = 0; idx < copy_entries.size(); ++idx) {
            std::time_t cur_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            if (cur_time - last_modified_time[idx] < std::get<2>(copy_entries[idx])) {
                continue;
            }
            for (const auto &file : std::filesystem::directory_iterator(std::get<0>(copy_entries[idx]))) {
                std::filesystem::rename(file.path(), std::get<1>(copy_entries[idx]) / file.path().filename());
            }
            cur_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            last_modified_time[idx] = cur_time;
        }
    }
}

void Daemon::Terminate(){
    syslog(LOG_NOTICE, "Daemon terminated");
    isRunning = false;
    closelog();
}

void Daemon::SignalHandler(int signum){
    switch(signum){
        case SIGINT:
            LoadConfig(configPath);
        case SIGTERM:
            Terminate();
            break;
        default:
            syslog(LOG_INFO, "Unknown signal found");
            break;
    }
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
    syslog(LOG_NOTICE, "The file has been read");
}