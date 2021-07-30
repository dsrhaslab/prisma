#include <iostream>
#include <cstdint>
#include <sys/mman.h>
#include <sys/types.h>
#include <thread>
#include <cstring>
#include <cstdlib>
#include <mutex>
#include <chrono>

#include "prisma.h"
#include "file_info.h"
#include "file_content.h"
#include "configs.h"

#include "ctpl.h"
#include <tbb/concurrent_hash_map.h>
//#include "third_party/ctpl/ctpl.h"
//#include "third_party/tbb/include/concurrent_hash_map.h"

// Initialize static variables
content_hash_map Prisma::buffer;
info_hash_map Prisma::files_info;
std::mutex Prisma::buffer_mutex;
std::mutex Prisma::initializer_mutex;
std::mutex Prisma::last_read_mutex;
std::condition_variable Prisma::buffer_free;
std::condition_variable Prisma::last_read;
int Prisma::counter = 0;
int Prisma::next_file = 0;
int Prisma::last_file = -1;
bool Prisma::first_call = true;
ctpl::thread_pool* Prisma::thread_pool;
Logger Prisma::logger;
Profiler Prisma::profiler;
Configs Prisma::configs;
Autotuner Prisma::autotuner;
bool Prisma::autotune_running = false;

void Prisma::set_configs() {
    // Read configuration file
    configs.read_config_file();

    // Print configurations
    configs.print_configs();
}

void Prisma::init() {
    // Set configs
    set_configs();

    if (configs.is_debug()) {
        // Open log
        logger.open();
        logger.write("Initializing PRISMA...");
    }

    // Initialize autotuner parameters
    autotuner.set_configs(&configs);
    autotuner.set_logger(&logger);
    autotuner.set_buffer_free(&buffer_free);

    // Start reader thread
    std::thread producer_thread(&Prisma::fill_buffer, this);
    producer_thread.detach();

    // Register handler for profiling
    if (configs.is_profiling()) {
        const int result_profiling = std::atexit(profiler.print_profiling);

        if (result_profiling != 0) {
            std::cerr << "[PRISMA] Profiling handler registration failed.";
            std::cerr << "[PRISMA] Profiling will not be performed.";
            configs.set_profiling(false);
        }
    }

    if (configs.is_debug()) {
        logger.write("Reader thread initialized.");
    }
}

std::string Prisma::get_filenames_path() {
    const char* home_env = std::getenv("HOME");

    if (home_env != nullptr) {
        return std::string(home_env) + "/prisma/filenames_list";
    } else {
        std::cerr << "[PRISMA] Could not find home directory." << std::endl;
        std::cerr << "[PRISMA] Filenames list not loaded." << std::endl;
        exit(1);
    }
}

bool Prisma::is_buffer_full() {
    return Prisma::buffer.size() >= configs.get_buffer_size();
}

void Prisma::autotune() {
    // Initial sleep
    usleep(configs.get_prev_autotune_sleep());
    // Start collecting buffer usage data
    autotune_running = true;
    usleep(configs.get_autotune_sleep());

    // Autotune cycle
    while (configs.is_buffer_autotune() || configs.is_threads_autotune()) {
        autotuner.tune();
        usleep(configs.get_autotune_sleep());
    }
}

FileInfo* Prisma::get_info(const std::string &filename) {
    info_hash_map::accessor a;
    bool new_file = Prisma::files_info.insert(a, filename);

    if (new_file) {
        auto *fi = new FileInfo(filename);
        a->second = fi;
    }

    FileInfo* res = a->second;

    // Release lock for this filename
    a.release();

    if (configs.is_debug() && new_file) {
        logger.write("Info for file '" + filename + "' created.");
    }

    return res;
}

void Prisma::read_file(FileInfo* fi) {
    // Get file iterator from buffer
    const std::string& filename = fi->get_filename();
    content_hash_map::const_accessor a;
    bool file_in_buffer = Prisma::buffer.find(a, filename);

    if(file_in_buffer) {
        FileContent* fc = a->second;

        // Release lock for this filename
        a.release();

        if (configs.is_debug()) {
            logger.write("Reading file '" + filename + "'...");
        }

        // Profiling - start time of the read operation
        auto start = std::chrono::steady_clock::now();

        // Read file
        bool success = fc->read_file(fi, configs.get_read_block_size());

        // Calculate read duration
        if (configs.is_profiling()) {
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            profiler.add_read_time(duration);
        }

        // Notify threads
        std::condition_variable* c = fi->get_cond_var().get();
        c->notify_all();

        if (configs.is_debug()) {
            logger.write("Consumer threads notified.");
        }

        // Check if file was successfully read
        if (success) {
            if (configs.is_debug()) {
                logger.write("File '" + filename + "' successfully read.");
            }
        } else {
            std::cerr << "[PRISMA] File '" + filename + "' couldn't be successfully read." << std::endl;
        }

        if (configs.is_debug()) {
            logger.write("Content of file '" + filename + "' stored in the buffer.");
        }
    } else {
        // Release lock for this filename
        a.release();
    }
}

void Prisma::wait_buffer_free(int thread_id, int file_id) {
    bool buffer_full;

    // Lock
    std::unique_lock<std::mutex> ul_buffer(buffer_mutex);

    // Wait while the buffer is full or it is not the thread's turn
    while ((buffer_full = is_buffer_full()) || file_id > next_file) {
        if (configs.is_debug()) {
            if (buffer_full) {
                logger.write("Thread " + std::to_string(thread_id) + " waiting for free space in the buffer.");
            } else {
                logger.write("Thread " + std::to_string(thread_id) + " waiting for its turn.");
            }
        }

        if (configs.is_profiling()) {
            // Profiling - increment number of waits for producing
            profiler.inc_wait_produce();
        }

        // Wait
        Prisma::buffer_free.wait(ul_buffer);
    }

    // Unlock
    ul_buffer.unlock();
}

void Prisma::store_file_content(const std::string &filename, FileInfo *fi) {
    // Check if buffer contains file
    content_hash_map::accessor a;
    bool new_file = buffer.insert(a, filename);

    if (!new_file) {
        // Increase number of expected file reads
        (a->second)->inc_n_reads();

        // Release lock for this filename
        a.release();

        if (configs.is_debug()) {
            logger.write("Buffer already contains the file '" + filename + "'.");
        }
    } else {
        // Add file with no content to buffer
        auto *fc = new FileContent(filename);
        a->second = fc;

        // Release lock for this filename
        a.release();

        // Save buffer usage for autotuning
        if (autotune_running) {
            autotuner.add_buffer_usage(buffer.size());
        }

        if (configs.is_debug()) {
            logger.write("File '" + filename + "' stored in the buffer with no content.");
            logger.write("Buffer has " + std::to_string(buffer.size()) + " elements.");
        }
    }

    // Lock
    buffer_mutex.lock();
    // Update next_file counter
    next_file++;
    // Unlock
    buffer_mutex.unlock();

    // Notify threads that are waiting on
    // this condition: file_id > next_file
    buffer_free.notify_all();

    if (new_file) {
        // Store file content
        read_file(fi);
    }
}

void Prisma::handle_file(int id, const std::string& filename, int file_id) {
    if (configs.is_debug()) {
        logger.write("Thread " + std::to_string(id) + " handling file '" + filename + "'.");
    }

    // Check if this is the last file
    if (file_id == Prisma::last_file){
        std::unique_lock<std::mutex> ul_last_read(last_read_mutex);
        // Wait for the last file to be read
        Prisma::last_read.notify_all();
        ul_last_read.unlock();
    }

    // Get info about file
    FileInfo* fi = get_info(filename);

    // Wait for buffer to be free
    wait_buffer_free(id, file_id);

    // Read file and store content if not in buffer
    store_file_content(filename, fi);
}

void Prisma::fill_buffer() {
    // Open filenames list
    std::string filenames_list_path = get_filenames_path();
    std::ifstream filenames_list(filenames_list_path);

    // Create thread pool with n_threads
    ctpl::thread_pool t_pool(configs.get_n_threads());
    Prisma::thread_pool = &t_pool;
    autotuner.set_thread_pool(&t_pool);

    if (configs.is_debug()) {
        logger.write("Thread pool created.");
    }

    // Start autotuner thread
    if (configs.is_buffer_autotune() || configs.is_threads_autotune()) {
        std::thread autotuner_thread(&Prisma::autotune, this);
        autotuner_thread.detach();

        if (configs.is_debug()) {
            logger.write("Autotuner thread launched.");
        }
    }

    if (configs.is_debug()) {
        logger.write("Starting to fill the read operations queue.");
    }
    if (filenames_list.is_open()) {
        std::string line;
        // Read one file at a time
        while(getline(filenames_list, line)) {
            int file_id = Prisma::counter;
            // Push file to thread pool's queue
            t_pool.push(handle_file, line, file_id);
            Prisma::counter++;
        }

        // Close filenames list
        filenames_list.close();

        // Store last file's ID
        Prisma::last_file = Prisma::counter - 1;
    }
    else if(!filenames_list.good()) {
        std::cerr << "[PRISMA] File '" + filenames_list_path + "' doesn't exist." << std::endl;
    }
    else {
        std::cerr << "[PRISMA] Could not open filenames list." << std::endl;
    }

    // Prevent thread pool destructor from being called
    // Note: the thread pool destructor calls stop()
    // and this does not allow the resize to be done
    std::unique_lock<std::mutex> ul_last_read(last_read_mutex);
    // Wait for the last file to be read
    Prisma::last_read.wait(ul_last_read);
    ul_last_read.unlock();

    // Stop autotuning thread
    configs.set_buffer_autotune(false);
    configs.set_threads_autotune(false);
    autotune_running = false;

    if (configs.is_debug()) {
        logger.write("Finishing execution...");
    }
}

char* Prisma::get_file_content(const std::string &filename, size_t n, uint64_t offset, size_t file_size) {
    content_hash_map::const_accessor a;
    bool file_in_buffer = Prisma::buffer.find(a, filename);

    if(file_in_buffer) {
        FileContent* fc = a->second;

        // Release lock for this filename
        a.release();

        return fc->get_content(n, offset, file_size);
    }
    else {
        // Release lock for this filename
        a.release();

        return nullptr;
    }
}

void Prisma::check_buffer_size(const std::string &filename, size_t n, uint64_t offset) {
    // Get info about file
    FileInfo* fi = get_info(filename);

    content_hash_map::accessor a;
    bool file_in_buffer = Prisma::buffer.find(a, filename);

    if (file_in_buffer) {
        FileContent* fc = a->second;

        // Check if the thread has finished reading the file
        if (offset + n >= fi->get_size()) {
            fc->dec_n_reads();
        }

        // Check if file will be read again
        if (fc->get_n_reads() == 0) {
            // Remove file from the buffer
            Prisma::buffer.erase(a);

            // Delete FileContent
            delete fc;

            buffer_mutex.lock();
            // Notify threads of free space in the buffer
            Prisma::buffer_free.notify_all();
            buffer_mutex.unlock();

            // Save buffer usage for autotuning
            if (autotune_running) {
                autotuner.add_buffer_usage(buffer.size());
            }

            if (configs.is_debug()) {
                logger.write("File '" + filename + "' removed from the buffer.");
                logger.write("Buffer has " + std::to_string(buffer.size()) + " elements.");
            }
        } else {
            // Release lock for this filename
            a.release();
        }
    } else {
        // Release lock for this filename
        a.release();
        std::cerr << "[PRISMA] File " << filename << " not in buffer." << std::endl;
    }
}

ssize_t Prisma::read(const std::string& filename, char* result, size_t n, uint64_t offset) {
    char* res;

    // Lock
    Prisma::initializer_mutex.lock();
    // Initialize producers
    if (first_call) {
        Prisma::first_call = false;
        init();
    }
    // Unlock
    Prisma::initializer_mutex.unlock();

    // Get info about file
    FileInfo* fi = get_info(filename);

    // If offset greater than file_size there's no need
    // to wait for file content
    size_t file_size = fi->get_size();
    if (offset >= file_size) {
        if (configs.is_debug()) {
            logger.write("Tried to read file '" + filename  + "' beyond the file size.");
        }

        return 0;
    }

    // Lock
    std::mutex* l = fi->get_mutex().get();
    std::unique_lock<std::mutex> ul(*l);

    // Wait if the buffer doesn't contain the file
    while((res = get_file_content(filename, n, offset, file_size)) == nullptr) {
        if (configs.is_debug()) {
            logger.write("Waiting for file '" + filename  + "' (off: "
                + std::to_string(offset) + ", size: " + std::to_string(n) + "').");
        }

        if (configs.is_profiling()) {
            // Profiling - increment number of waits for consuming
            profiler.inc_wait_consume();
        }

        // Wait
        std::condition_variable* c = fi->get_cond_var().get();
        c->wait(ul);
    }

    // Check if (offset + n) exceeds the limits
    // of the res array
    if (offset + n > file_size) {
        n = file_size - offset;
    }

    // Save content in result array
    memcpy(result, res + offset, n);

    // Unlock
    ul.unlock();

    if (configs.is_debug()) {
        logger.write("Read file '" + filename  + "' (off: "
                 + std::to_string(offset) + ", size: " + std::to_string(n) + ") from buffer.");
    }

    // Check buffer size
    check_buffer_size(filename, n, offset);

    return n;
}
