#include <cmath>
#include <iostream>
#include <utility>
#include <cmath>
#include <cstring>

#include "autotuner.h"

void Autotuner::tune() {
    if (configs->is_debug()) {
        logger->write("Autotuning...");
    }

    // Check if there are enough values to calculate the
    // average of the buffer usage list
    if (buffer_usage_list.size() >= 10) {
        // buffer_usage = (avg, std_dev)
        std::pair<int, int> avg_buffer_usage = get_avg_buffer_usage();

        if (configs->is_debug()) {
            logger->write("Buffer usage: " + std::to_string(avg_buffer_usage.first) +
                          " \u00b1 " + std::to_string(avg_buffer_usage.second) + ".");
        }

        if (avg_buffer_usage.first >= 80 && avg_buffer_usage.second <= 20) {
            // Turn off autotune if optimal solution was found
            configs->set_buffer_autotune(false);
            configs->set_threads_autotune(false);

            if (configs->is_debug()) {
                logger->write(
                    "Autotune turned off");
            }
        } else if (avg_buffer_usage.first <= 30 || avg_buffer_usage.second <= 20) {
            // Increase number of threads if std deviation is lower than 20%
            inc_n_threads();
        } else {
            // Increase buffer size
            inc_buffer_size();
        }

        // Increase autotune sleep
        /*configs->set_autotune_sleep(
                std::min(configs->get_autotune_sleep() * 2, configs->get_max_autotune_sleep()));

        if (configs->is_debug()) {
            logger->write(
                    "Autotune sleep time increased for " + std::to_string(configs->get_autotune_sleep()) +
                    " ns.");
        }*/

        // Clean buffer_usage_list
        buffer_usage_list.clear();
    }
}

void Autotuner::inc_n_threads() const {
    if (configs->is_threads_autotune()) {
        if (configs->get_n_threads() < configs->get_max_n_threads()) {
            configs->inc_n_threads();
            thread_pool->resize(configs->get_n_threads());

            if (configs->is_debug()) {
                logger->write("Increasing nr of threads for " + std::to_string(configs->get_n_threads()) + ".");
            }

            std::cout <<"[PRISMA] Increasing nr of threads for " + std::to_string(configs->get_n_threads()) + "." << std::endl;
        } else {
            if (configs->is_debug()) {
                logger->write("Nr of threads reached maximum value.");
            }
        }
    }
}

void Autotuner::inc_buffer_size() const {
    if(configs->is_buffer_autotune()) {
        if (configs->get_buffer_size() < configs->get_max_buffer_size()) {
            configs->inc_buffer_size();
            // Notify threads that are waiting for free space in buffer
            buffer_free->notify_all();

            if (configs->is_debug()) {
                logger->write("Increasing buffer size for " + std::to_string(configs->get_buffer_size()) + ".");
            }

            std::cout <<"[PRISMA] Increasing buffer size for " + std::to_string(configs->get_buffer_size()) + "." << std::endl;
        } else {
            if (configs->is_debug()) {
                logger->write("Buffer size reached maximum value.");
            }
        }
    }
}

void Autotuner::add_buffer_usage(int n_elems) {
    float buffer_usage = ((float) n_elems / (float) configs->get_buffer_size()) * 100;
    buffer_usage_list.push_back((int) buffer_usage);
}

std::pair<int, int> Autotuner::get_avg_buffer_usage() {
    std::pair<int, int> avg_buffer_usage;
    float sum = 0.0, avg, std_dev = 0.0;

    for(int b : buffer_usage_list) {
        sum += b;
    }

    avg = sum / buffer_usage_list.size();
    avg_buffer_usage.first = (int) avg;

    for(int b : buffer_usage_list) {
        std_dev += std::pow(b - avg, 2);
    }

    avg_buffer_usage.second = (int) std::sqrt(std_dev / buffer_usage_list.size());

    return avg_buffer_usage;
}

void Autotuner::set_thread_pool(ctpl::thread_pool *tp) {
    thread_pool = tp;
}

void Autotuner::set_configs(Configs *c) {
    Autotuner::configs = c;
}

void Autotuner::set_logger(Logger *l) {
    Autotuner::logger = l;
}

void Autotuner::set_buffer_free(std::condition_variable *bf) {
    Autotuner::buffer_free = bf;
}