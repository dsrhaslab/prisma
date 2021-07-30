#include <iostream>
#include <ctime>
#include <cstring>
#include <sys/stat.h>

#include "logger.h"

void Logger::open() {
    // Get log path
    const char* home_env = std::getenv("HOME");
    time_t t = std::time(0);
    auto timestamp = static_cast<long int> (t);
    std::string debug_dir = std::string(home_env) + "/prisma/debug";

    // Create debug directory
    int dir_res = mkdir(debug_dir.c_str(), 0777);
    if (dir_res == 0) {
        std::cout << "[PRISMA] Debug directory created." << std::endl;
    }

    // Open log file
    if (home_env != nullptr) {
        log_path = std::string(home_env) + "/prisma/debug/run-" + std::to_string(timestamp) + ".log";
        log.open(log_path, std::ios_base::app);
    } else {
        std::cerr << "[PRISMA] Could not find home directory.\n Debug will not be performed." << std::endl;
    }
}

void Logger::write(const std::string &msg) {
    // current date/time based on current system
    time_t now = time(0);
    // convert now to string form
    char* timestamp = ctime(&now);
    if (timestamp[strlen(timestamp) - 1] == '\n') timestamp[strlen(timestamp) - 1] = '\0';

    log_mutex.lock();
    log << "[" << timestamp << "] " << msg << std::endl;
    log_mutex.unlock();
}