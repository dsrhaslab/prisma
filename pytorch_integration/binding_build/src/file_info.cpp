#include <utility>
#include <cstring>
#include <fstream>
#include <iostream>

#include "file_info.h"

FileInfo::FileInfo(std::string f) {
    filename = std::move(f);
    size = get_file_size();
    mutex = static_cast<std::unique_ptr<std::mutex>>(new std::mutex);
    cond_var = static_cast<std::unique_ptr<std::condition_variable>>(new std::condition_variable);
}

size_t FileInfo::get_file_size() {
    std::ifstream in_file(filename, std::ios::binary | std::ios::ate);
    return in_file.tellg();
}

const std::string &FileInfo::get_filename() const {
    return filename;
}

void FileInfo::set_filename(const std::string &f) {
    FileInfo::filename = f;
}

size_t FileInfo::get_size() const {
    return size;
}

void FileInfo::set_size(size_t s) {
    FileInfo::size = s;
}

const std::unique_ptr<std::mutex> &FileInfo::get_mutex() const {
    return mutex;
}

const std::unique_ptr<std::condition_variable> &FileInfo::get_cond_var() const {
    return cond_var;
}
