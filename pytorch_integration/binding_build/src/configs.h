#ifndef PRISMA_CONFIGS_H
#define PRISMA_CONFIGS_H

#include <string>

class Configs {
private:
    std::string configs_path;
    long autotune_sleep;
    long max_autotune_sleep;
    long prev_autotune_sleep;
    int buffer_size;
    int n_threads;
    int buffer_size_step;
    int n_threads_step;
    int init_buffer_size;
    int init_n_threads;
    int max_buffer_size;
    int max_n_threads;
    bool buffer_autotune;
    bool threads_autotune;
    size_t read_block_size;
    bool profiling;
    bool debug;

    void add_config(const std::string& name, const std::string& value);

public:
    Configs();
    ~Configs(){};

    void print_configs() const;
    void read_config_file();
    void inc_buffer_size();
    void dec_buffer_size();
    void inc_n_threads();
    void dec_n_threads();
    int get_buffer_size() const;
    void set_buffer_size(int bs);
    int get_n_threads() const;
    void set_n_threads(int nt);
    size_t get_read_block_size() const;
    void set_read_block_size(size_t rbs);
    bool is_profiling() const;
    void set_profiling(bool p);
    bool is_debug() const;
    void set_debug(bool d);
    int get_max_buffer_size() const;
    void set_max_buffer_size(int max_bs);
    int get_max_n_threads() const;
    void set_max_n_threads(int max_nt);
    bool is_buffer_autotune() const;
    void set_buffer_autotune(bool b);
    bool is_threads_autotune() const;
    void set_threads_autotune(bool b);
    long get_autotune_sleep() const;
    void set_autotune_sleep(long as);
    long get_max_autotune_sleep() const;
    void set_max_autotune_sleep(long max_as);
    long get_prev_autotune_sleep() const;
    void set_prev_autotune_sleep(long prev_as);
};


#endif //PRISMA_CONFIGS_H
