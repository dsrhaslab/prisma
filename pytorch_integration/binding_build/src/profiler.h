#ifndef PRISMA_PROFILER_H
#define PRISMA_PROFILER_H


#include <list>
#include <mutex>

class Profiler {
private:
    static std::list<long> read_time;
    static int nr_producer_waits;
    static int nr_consumer_waits;
    std::mutex mutex;

public:
    Profiler(){};
    ~Profiler(){};

    void inc_wait_consume();
    void inc_wait_produce();
    void add_read_time(long duration);
    static void print_profiling();
};


#endif //PRISMA_PROFILER_H
