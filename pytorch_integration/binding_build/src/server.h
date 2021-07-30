#ifndef PRISMA_SERVER_H
#define PRISMA_SERVER_H

#include "prisma.h"

class Server {
private:
    struct sockaddr_un unix_socket;
    int server_fd;
    int addr_len;
    Prisma prisma;

    void create_connection();
    int accept();
    void handle_request(int sock);

public:
    Server(){}
    ~Server(){}

    void run();
};


#endif //PRISMA_SERVER_H
