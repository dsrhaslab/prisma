#ifndef PRISMA_FILE_CONTENT_H
#define PRISMA_FILE_CONTENT_H


#include <string>
#include <mutex>
#include <condition_variable>
#include "file_info.h"

class FileContent {
private:
    std::string filename;
    int n_reads;
    char* content;
    uint64_t offset;

    ssize_t read_block(int fd, char *result, size_t n, uint64_t off);

public:
    FileContent(std::string f);
    ~FileContent();

    void inc_n_reads();
    void dec_n_reads();
    const std::string &get_filename() const;
    void set_filename(const std::string &f);
    int get_n_reads() const;
    void set_n_reads(int n);
    char *get_content() const;
    void set_content(char *content);
    bool read_file(FileInfo *fi, size_t block_size);
    char *get_content(size_t n, uint64_t off, size_t file_size) const;
};


#endif //PRISMA_FILE_CONTENT_H
