#ifndef PRISMA_FILE_INFO_H
#define PRISMA_FILE_INFO_H

#include <string>
#include <mutex>
#include <condition_variable>

class FileInfo {
private:
    std::string filename;
    size_t size;
    std::unique_ptr<std::mutex> mutex;
    std::unique_ptr<std::condition_variable> cond_var;

    size_t get_file_size();

public:
    FileInfo(std::string f);
    ~FileInfo(){};

    const std::string &get_filename() const;
    void set_filename(const std::string &f);
    size_t get_size() const;
    void set_size(size_t s);
    const std::unique_ptr<std::mutex> &get_mutex() const;
    const std::unique_ptr<std::condition_variable> &get_cond_var() const;
};


#endif //PRISMA_FILE_INFO_H
