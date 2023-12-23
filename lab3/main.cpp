#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>
#include <sys/syslog.h>
#include "test/test.h"


int main(int argc, char** argv){
    if (argc != 2) {
        logMessage("ERROR: Incorrect number of arguments. Check the config file.", LOG_ERR);
        return EXIT_FAILURE;
    }
    
    std::ifstream configFile(argv[1]);
    if (!configFile.is_open()) {
        logMessage("ERROR: Invalid config", LOG_NOTICE);
        return EXIT_SUCCESS;
    }

    std::regex pushTestFmt(R"(^\s*Push:\s*(\d+)\s*(\d+)\s*(\d+)\s*$)"),
               popTestFmt(R"(^\s*Pop:\s*(\d+)\s*(\d+)\s*(\d+)\s*$)"),
               pushPopTestFmt(R"(^\s*PushPop:\s*(\d+)\s*(\d+)\s*$)"),
               emptyFmt(R"(^\s*$)");

    std::string line;
    while (std::getline(configFile, line)) {
        if (std::regex_match(line, emptyFmt)) {
            continue;
        } else if (std::regex_match(line, pushTestFmt)) {
            std::vector<std::smatch> matches(std::sregex_iterator(line.begin(), line.end(), pushTestFmt), std::sregex_iterator());
            runPushTest(matches);
        } else if (std::regex_match(line, popTestFmt)) {
            std::vector<std::smatch> matches(std::sregex_iterator(line.begin(), line.end(), popTestFmt), std::sregex_iterator());
            runPopTest(matches);
        } else if (std::regex_match(line, pushPopTestFmt)) {
            std::vector<std::smatch> matches(std::sregex_iterator(line.begin(), line.end(), pushPopTestFmt), std::sregex_iterator());
            runPushPopTest(matches);
        } else {
            logMessage("ERROR: Failed to parse string: \"" + line + "\"\nIt will be ignored", LOG_ERR);
        }
    }

    return EXIT_SUCCESS;
}