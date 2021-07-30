#ifndef PRISMA_LOGGER_H
#define PRISMA_LOGGER_H


#include <mutex>
#include <fstream>

class Logger {
private:
    std::string log_path;
    std::ofstream log;
    std::mutex log_mutex;

public:
    Logger(){};
    ~Logger(){};

    void write(const std::string& msg);
    void open();
};


#endif //PRISMA_LOGGER_H
