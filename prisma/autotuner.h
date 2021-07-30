#ifndef PRISMA_AUTOTUNER_H
#define PRISMA_AUTOTUNER_H


#include "configs.h"
#include "logger.h"
#include "profiler.h"
#include <list>
#include <condition_variable>
#include <queue>

#include "ctpl.h"
//#include "third_party/ctpl/ctpl.h"

class Autotuner {
private:
    Configs *configs;
    ctpl::thread_pool *thread_pool;
    Logger *logger;
    std::condition_variable* buffer_free;
    std::list<int> buffer_usage_list;

    void inc_buffer_size() const;
    void inc_n_threads() const;
    std::pair<int, int> get_avg_buffer_usage();

public:
    Autotuner(){};
    ~Autotuner(){};

    void set_configs(Configs *c);
    void set_thread_pool(ctpl::thread_pool *tp);
    void set_logger(Logger *l);
    void set_buffer_free(std::condition_variable* bf);
    void add_buffer_usage(int n_elems);
    void tune();
};


#endif //PRISMA_AUTOTUNER_H
