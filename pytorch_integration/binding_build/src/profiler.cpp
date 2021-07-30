#include <iostream>
#include <numeric>
#include "profiler.h"

std::list<long> Profiler::read_time;
int Profiler::nr_producer_waits = 0;
int Profiler::nr_consumer_waits = 0;

void Profiler::inc_wait_consume() {
    mutex.lock();
    nr_consumer_waits ++;
    mutex.unlock();
}

void Profiler::inc_wait_produce() {
    mutex.lock();
    nr_producer_waits++;
    mutex.unlock();
}

void Profiler::add_read_time(long duration) {
    mutex.lock();
    read_time.push_back(duration);
    mutex.unlock();
}

void Profiler::print_profiling() {
    std::cout << "==============================" << std::endl;
    std::cout << "  [PRISMA PROFILING]  \n" << std::endl;

    if (!read_time.empty()) {
        std::cout << "  Read duration: "
                     + std::to_string((int) std::accumulate(read_time.begin(), read_time.end(), 0.0) / read_time.size())
                     + " ms" << std::endl;
    } else {
        std::cout << "  Read duration: ERROR - No files read!" << std::endl;
    }

    std::cout << "  #Producer waits: " + std::to_string(nr_producer_waits) << std::endl;
    std::cout << "  #Consumer waits: " + std::to_string(nr_consumer_waits) << std::endl;
    std::cout << "==============================\n" << std::endl;
}
