#ifndef PRISMA_PRISMA_H
#define PRISMA_PRISMA_H

#include <unordered_map>
#include <string>
#include <mutex>
#include <list>
#include <condition_variable>
#include <boost/functional/hash.hpp>

#include "file_info.h"
#include "file_content.h"
#include "logger.h"
#include "profiler.h"
#include "configs.h"
#include "autotuner.h"

#include "ctpl.h"
#include <tbb/concurrent_hash_map.h>
//#include "third_party/ctpl/ctpl.h"
//#include "third_party/tbb/include/concurrent_hash_map.h"

template<typename K>
struct HashCompare {
    static size_t hash( const K& key )                  { return boost::hash_value(key); }
    static bool   equal( const K& key1, const K& key2 ) { return ( key1 == key2 ); }
};

typedef tbb::concurrent_hash_map<std::string, FileContent*, HashCompare<std::string>> content_hash_map;
typedef tbb::concurrent_hash_map<std::string, FileInfo*, HashCompare<std::string>> info_hash_map;

class Prisma {
private:
    static content_hash_map buffer;
    static info_hash_map files_info;
    static std::mutex buffer_mutex;
    static std::mutex initializer_mutex;
    static std::mutex last_read_mutex;
    static std::condition_variable buffer_free;
    static std::condition_variable last_read;
    static int counter;
    static int next_file;
    static int last_file;
    static bool first_call;
    static ctpl::thread_pool* thread_pool;
    static Logger logger;
    static Configs configs;
    static Profiler profiler;
    static Autotuner autotuner;
    static bool autotune_running;

    void fill_buffer();
    static std::string get_filenames_path();
    static void read_file(FileInfo* fi);
    char* get_file_content(const std::string &filename, size_t n, uint64_t offset, size_t file_size);
    static void wait_buffer_free(int thread_id, int file_id);
    static void store_file_content(const std::string &filename, FileInfo *fi);
    static void handle_file(int id, const std::string &filename, int file_id);
    static bool is_buffer_full();
    void check_buffer_size(const std::string &filename, size_t n, uint64_t offset);
    static FileInfo *get_info(const std::string &filename);
    void set_configs();
    void autotune();

public:
    Prisma(){}
    ~Prisma(){}

    ssize_t read(const std::string& filename, char* result, size_t n, uint64_t offset);
    void init();
};


#endif //PRISMA_PRISMA_H
