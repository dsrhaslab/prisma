#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <thread>
#include <cstring>

#include "configs.h"

Configs::Configs() {
    // Get configs path
    const char* home_env = std::getenv("HOME");

    if (home_env != nullptr) {
        configs_path = std::string(home_env) + "/prisma/configs";
    } else {
        std::cerr << "[PRISMA] Could not find home directory." << std::endl;
    }

    // Default values
    buffer_size = 10;
    n_threads = 1;
    init_buffer_size = 10;
    init_n_threads = 1;
    buffer_size_step = 10;
    n_threads_step = 1;
    read_block_size = 65536;
    buffer_autotune = true;
    threads_autotune = true;
    max_buffer_size = 100;
    max_n_threads = std::thread::hardware_concurrency();
    autotune_sleep = 100000;
    max_autotune_sleep = 3600000000;
    prev_autotune_sleep = 15000000;
    profiling = false;
    debug = false;
}

void Configs::add_config(const std::string &name, const std::string &value) {
    if (!value.empty()) {
        if (name == "buffer_size") {
            if (value != "autotune") {
                buffer_size = stoi(value);
                buffer_autotune = false;
            }
        } else if (name == "n_threads") {
            if (value != "autotune") {
                n_threads = stoi(value);
                threads_autotune = false;
            }
        } else if (name == "max_buffer_size") {
            max_buffer_size = stoi(value);
        } else if (name == "max_n_threads") {
            max_n_threads = stoi(value);
        } else if (name == "init_buffer_size") {
            init_buffer_size = stoi(value);
        } else if (name == "init_n_threads") {
            init_n_threads = stoi(value);
        } else if (name == "buffer_size_step") {
            buffer_size_step = stoi(value);
        } else if (name == "n_threads_step") {
            n_threads_step = stoi(value);
        } else if (name == "read_block_size") {
            read_block_size = stoi(value);
        } else if (name == "autotune_interval") {
            autotune_sleep = stoi(value)*1000;
        } else if (name == "profiling") {
            profiling = value == "true";
        } else if (name == "debug") {
            debug = value == "true";
        }
    }
}

void Configs::read_config_file() {
    // Open configs file
    std::ifstream config_file(configs_path);

    if (config_file.is_open()) {
        std::string line;
        while(getline(config_file, line)) {
            // Clean whitespaces
            line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());

            // Ignore comments
            if(line[0] == '#' || line.empty())
                continue;

            // Parse line
            auto delimiter_pos = line.find('=');
            auto name = line.substr(0, delimiter_pos);
            auto value = line.substr(delimiter_pos + 1);

            // Store config
            Configs::add_config(name, value);
        }

        // Close configs file
        config_file.close();

        // Update initial values
        if (buffer_autotune) {
            buffer_size = init_buffer_size;
        }
        if (threads_autotune) {
            n_threads = init_n_threads;
        }

    } else {
        std::cerr << "[PRISMA] Could not open configuration file." << std::endl;
        std::cerr << "[PRISMA] The default configurations will be used." << std::endl;
    }
}

void Configs::print_configs() const {
    // Print configurations
    std::cout << "==============================" << std::endl;
    std::cout << "  [PRISMA CONFIGURATIONS]  \n" << std::endl;

    if (buffer_autotune) {
        std::cout << "  Buffer size: autotune" << std::endl;
        std::cout << "  Initial buffer size: " + std::to_string(init_buffer_size) << std::endl;
        std::cout << "  Buffer size step: " + std::to_string(buffer_size_step) << std::endl;
        std::cout << "  Maximum buffer size: " + std::to_string(max_buffer_size) << std::endl;
    } else {
        std::cout << "  Buffer size: " + std::to_string(buffer_size) << std::endl;
    }

    if(threads_autotune) {
        std::cout << "  Nr of threads: autotune" << std::endl;
        std::cout << "  Initial nr of threads: " + std::to_string(init_n_threads) << std::endl;
        std::cout << "  Nr of threads step: " + std::to_string(n_threads_step) << std::endl;
        std::cout << "  Maximum nr of threads: " + std::to_string(max_n_threads) << std::endl;
    } else {
        std::cout << "  Nr of threads: " + std::to_string(n_threads) << std::endl;
    }

    if (buffer_autotune or threads_autotune) {
        std::cout << "  Autotuning interval: " + std::to_string(autotune_sleep) + " ms" << std::endl;
    }

    std::cout << "  Read block size: " + std::to_string(read_block_size) << std::endl;
    std::cout << "  Profiling: " + std::to_string(profiling) << std::endl;
    std::cout << "  Debug: " + std::to_string(debug) << std::endl;
    std::cout << "==============================\n" << std::endl;
}

void Configs::inc_buffer_size() {
    buffer_size += buffer_size_step;
}

void Configs::dec_buffer_size() {
    buffer_size -= buffer_size_step;
}

void Configs::inc_n_threads() {
    n_threads += n_threads_step;
}

void Configs::dec_n_threads() {
    n_threads -= n_threads_step;
}

int Configs::get_buffer_size() const {
    return buffer_size;
}

void Configs::set_buffer_size(int bs) {
    buffer_size = bs;
}

int Configs::get_n_threads() const {
    return n_threads;
}

void Configs::set_n_threads(int nt) {
    n_threads = nt;
}

size_t Configs::get_read_block_size() const {
    return read_block_size;
}

void Configs::set_read_block_size(size_t rbs) {
    read_block_size = rbs;
}

bool Configs::is_buffer_autotune() const {
    return buffer_autotune;
}

void Configs::set_buffer_autotune(bool ba) {
    Configs::buffer_autotune = ba;
}

bool Configs::is_threads_autotune() const {
    return threads_autotune;
}

void Configs::set_threads_autotune(bool ta) {
    Configs::threads_autotune = ta;
}

bool Configs::is_profiling() const {
    return profiling;
}

void Configs::set_profiling(bool p) {
    Configs::profiling = p;
}

bool Configs::is_debug() const {
    return debug;
}

void Configs::set_debug(bool d) {
    Configs::debug = d;
}

int Configs::get_max_buffer_size() const {
    return max_buffer_size;
}

void Configs::set_max_buffer_size(int max_bs) {
    max_buffer_size = max_bs;
}

int Configs::get_max_n_threads() const {
    return max_n_threads;
}

void Configs::set_max_n_threads(int max_nt) {
    max_n_threads = max_nt;
}

long Configs::get_autotune_sleep() const {
    return autotune_sleep;
}

void Configs::set_autotune_sleep(long as) {
    autotune_sleep = as;
}

long Configs::get_max_autotune_sleep() const {
    return max_autotune_sleep;
}

void Configs::set_max_autotune_sleep(long max_as) {
    max_autotune_sleep = max_as;
}

long Configs::get_prev_autotune_sleep() const {
    return prev_autotune_sleep;
}

void Configs::set_prev_autotune_sleep(long prev_as) {
    prev_autotune_sleep = prev_as;
}
