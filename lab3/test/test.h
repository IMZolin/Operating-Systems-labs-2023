#pragma once
#include <regex>
#include <sys/syslog.h>

#include "../queue/mcmp_queue.hpp"

void logMessage(const std::string& message, int priority);
bool pushTest(size_t threadNum, int numberForOneThread, int repeatNum);
bool popTest(size_t threadNum, int number, int repeatNum);
bool pushPopTest(int number, int repeatNum);
void runPushTest(const std::vector<std::smatch>& matches);
void runPopTest(const std::vector<std::smatch>& matches);
void runPushPopTest(const std::vector<std::smatch>& matches);