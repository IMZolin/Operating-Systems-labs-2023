#pragma once
#include <string>
#include <chrono>
#include <csignal>
#include <syslog.h>
#include <filesystem>
#include <vector>
#include <tuple>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

using config_entriers = std::vector<std::tuple<std::filesystem::path, std::filesystem::path, std::time_t>>; 

class Daemon{
    public:
        Daemon() = delete;
        Daemon(const Daemon &) = delete;
        Daemon(Daemon &&) = delete;
        ~Daemon() = default;

        static Daemon &getInstance(const std::string &path_to_config);
        void Run() const;
    protected:
        static void SignalHandler(int signum);
        void CloseRunning() const;
        void DoFork() const;
    private:
        Daemon(const std::string &path_to_config);
        static void LoadConfig(const std::string &config_file);
        static std::string configPath;
        static config_entriers entries;
        std::string pidFilePath = std::filesystem::absolute("/var/run/mydaemon.pid");
        const std::string procDir = "/proc";
        const std::string syslogProcName = "mydaemon";
};