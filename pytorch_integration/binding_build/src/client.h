#ifndef PRISMA_CLIENT_H
#define PRISMA_CLIENT_H

#include <sys/un.h>
#include <string>

#define MAX_FILENAME_SIZE 1000

class Client {
private:
    int sock;
    struct sockaddr_un unix_socket;

    void create_connection();

public:
    struct request {
        char filename[MAX_FILENAME_SIZE];
        size_t n_bytes;
        uint64_t offset;
    };

    Client();
    ~Client(){}

    ssize_t read(const std::string& filename, char* result, size_t n, uint64_t offset);
};


#endif //PRISMA_CLIENT_H
