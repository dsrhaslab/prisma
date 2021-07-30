#include <utility>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

#include "file_content.h"
#include "file_info.h"

FileContent::FileContent(std::string f) {
    filename = std::move(f);
    n_reads = 1;
    content = nullptr;
    offset = 0;
}

FileContent::~FileContent() {
    delete[] content;
}

ssize_t FileContent::read_block(int fd, char* result, size_t n, uint64_t off) {
    int error = 0;
    char* dst = result;
    ssize_t bytes_read = 0;

    while (n > 0 && error == 0) {
        // Some platforms, notably macs, throw EINVAL if pread is asked to read
        // more than fits in a 32-bit integer.
        size_t requested_read_length;
        if (n > INT32_MAX) {
            requested_read_length = INT32_MAX;
        } else {
            requested_read_length = n;
        }

        ssize_t r = pread(fd, dst, requested_read_length, static_cast<off_t>(off));

        if (r > 0) {
            dst += r;
            n -= r;
            bytes_read += r;
            off += r;
        } else if (r == 0) {
            std::cerr << "[PRISMA] System call pread returned less bytes than requested." << std::endl;
            error = 1;
        } else if (errno == EINTR || errno == EAGAIN) {
            // Retry
        } else {
            std::cerr << "[PRISMA] IOError (off:" << off << ", size:" << requested_read_length
            << "): " << std::strerror(errno) << std::endl;
            error = 1;
        }
    }

    return bytes_read;
}

bool FileContent::read_file(FileInfo* fi, size_t block_size) {
    ssize_t bytes_read = 1;
    size_t file_size = fi->get_size();
    char* result_buffer = new char[block_size];
    std::mutex* l = fi->get_mutex().get();

    // Initialize content
    content = new char[file_size];

    // Update read_block_size size to prevent errors
    if(block_size > file_size) {
        block_size = file_size;
    }

    // Open file
    int fd = open(filename.c_str(), O_RDONLY);

    // Read file
    while(offset < file_size && bytes_read > 0) {
        // Read read_block_size
        bytes_read = read_block(fd, result_buffer, block_size, offset);

        // Lock
        std::unique_lock<std::mutex> ul(*l);

        // Store result
        memcpy(content + offset, result_buffer, bytes_read);
        // Update offset
        offset += bytes_read;

        // Unlock
        ul.unlock();

        // Update read_block_size size when TFRecord is reaching EOF
        if(file_size - offset < block_size) {
            block_size = file_size - offset;
        }
    }

    // Free temporary array
    delete[] result_buffer;

    // Close file descriptor
    close(fd);

    // Validate result
    return bytes_read > 0;
}

char *FileContent::get_content(size_t n, uint64_t off, size_t file_size) const {
    if (off + n <= offset || off + n > file_size) {
        return content;
    }

    return nullptr;
}

void FileContent::inc_n_reads() {
    n_reads += 1;
}

void FileContent::dec_n_reads() {
    if (n_reads >= 0) {
        n_reads -= 1;
    }
}

const std::string &FileContent::get_filename() const {
    return filename;
}

void FileContent::set_filename(const std::string &f) {
    FileContent::filename = f;
}

int FileContent::get_n_reads() const {
    return n_reads;
}

void FileContent::set_n_reads(int n) {
    n_reads = n;
}

char *FileContent::get_content() const {
    return content;
}

void FileContent::set_content(char *content) {
    FileContent::content = content;
}
