#include "Daemon.hpp"

std::string Daemon::configPath;
config_entriers Daemon::entries;

Daemon::Daemon(const std::string &path_to_config){
    if (std::filesystem::exists(path_to_config)){
        configPath = std::filesystem::absolute(path_to_config);
        Daemon::LoadConfig(configPath);
    }
    else{
        openlog(syslogProcName.c_str(), LOG_PID, LOG_DAEMON);
        syslog(LOG_NOTICE, "Config file does not exist. Daemon terminated");
        closelog();
        exit(EXIT_FAILURE);
    }
}

Daemon &Daemon::getInstance(const std::string &path_to_config){
    static Daemon daemon(path_to_config);
    return daemon;
}

void Daemon::Run() const{
    CloseRunning();
    DoFork();
    syslog(LOG_NOTICE, "Daemon started");
    std::vector<std::time_t> last_modified_time(entries.size(), 0);
    while(true){
        const std::vector<std::tuple<std::filesystem::path, std::filesystem::path, std::time_t>> & copy_entries = entries;
        for (size_t idx = 0; idx < copy_entries.size(); ++idx) {
            std::time_t cur_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            if (cur_time - last_modified_time[idx] < std::get<2>(copy_entries[idx])) {
                continue;
            }
            try{
                for (const auto &file : std::filesystem::directory_iterator(std::get<0>(copy_entries[idx]))) {
                    std::filesystem::rename(file.path(), std::get<1>(copy_entries[idx]) / file.path().filename());
                }
            }
            catch (const std::runtime_error& e) {
                syslog(LOG_NOTICE, "Config has invalid string");
                std::cerr << e.what() << std::endl;
                continue;
            }
            
            cur_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            last_modified_time[idx] = cur_time;
        }
    }
}

void Daemon::SignalHandler(int signum){
    switch(signum){
        case SIGHUP:
            Daemon::LoadConfig(configPath);
            break;
        case SIGTERM:
            syslog(LOG_NOTICE, "Daemon terminated");
            closelog();
            exit(EXIT_SUCCESS);
            break;
        default:
            syslog(LOG_INFO, "Unknown signal found");
            break;
    }
}

void Daemon::CloseRunning() const{
    int pid;
    std::ifstream f(pidFilePath);
    f >> pid;
    if (std::filesystem::exists(procDir + "/" + std::to_string(pid)))
        kill(pid, SIGTERM);
}

void Daemon::DoFork() const{
    pid_t pid = fork();
    if (pid != 0)
        exit(EXIT_FAILURE);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    std::signal(SIGHUP, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    pid = fork();
    if (pid != 0)
        exit(EXIT_FAILURE);

    umask(0);
    if (chdir("/") != 0)
        exit(EXIT_FAILURE);

    for (long x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
        close(x);
    openlog(syslogProcName.c_str(), LOG_PID, LOG_DAEMON);

    std::ofstream f(pidFilePath, std::ios_base::trunc);
    f << getpid();
}

void Daemon::LoadConfig(const std::string &config_file){
    entries.clear();
    std::ifstream f(configPath);
        std::string dir1, dir2, time;
        while (f >> dir1 >> dir2 >> time) {
            entries.push_back(std::make_tuple(std::filesystem::path(dir1), std::filesystem::path(dir2), std::time_t(std::stoi(time))));
        }
}