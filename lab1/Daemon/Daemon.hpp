#pragma once
#include <string>
#include <csignal>
#include <syslog.h>
#include <filesystem>
#include <vector>
#include <tuple>
#include <fstream>

using config_entriers = std::vector<std::tuple<std::filesystem::path, std::filesystem::path, std::time_t>>; 

class Daemon{
    public:
        Daemon(const Daemon &) = delete;
        Daemon(Daemon &&) = delete;
        ~Daemon() = default;

        static Daemon& getInstance(){
            static Daemon instance;
            return instance;
        }
        void Run();
        void Terminate();
        void LoadConfig(const std::string &config_file);
        void SignalHandler(int signum);
        void killPid();
    private:
        Daemon() = default;
        void SetConfigPath(const std::string &config_path);
        static std::string configPath;
        config_entriers entries;
        bool isRunning = false;
        std::string pidFilePath;
        const std::string procDir = "/proc";
        const std::string syslogProcName = "mydaemon";
};