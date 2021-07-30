#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <atomic>
#include <iostream>

#include "client.h"

Client::Client() {
    // Create UNIX connection with the PRISMA server
    create_connection();
}

void Client::create_connection(){
    unix_socket.sun_family = AF_UNIX;

    // Create socket name
    char socket_name[] = "prisma-socket";
    strncpy(unix_socket.sun_path, socket_name, sizeof(unix_socket.sun_path) - 1);

    // Create socket
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        std::cerr << "[PRISMA] Socket creation error: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        std::cout << "[PRISMA] Client socket created." << std::endl;
    }

    // Establish connection with PRISMA server
    if (::connect (sock, (const struct sockaddr *) &unix_socket, sizeof(struct sockaddr_un)) < 0) {
        std::cerr << "[PRISMA] Connection failed: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        std::cout << "[PRISMA] Client connection created for " << socket_name << "." << std::endl;
    }
}

ssize_t Client::read(const std::string &filename, char *result, size_t n, uint64_t offset) {
    // Send read request to PRISMA server
    request req{};
    strcpy(req.filename, filename.c_str());
    req.n_bytes = n;
    req.offset = offset;

    int return_value_t = ::write(sock, &req, sizeof(struct request));

    if (return_value_t != sizeof(struct request)) {
        std::cerr << "[PRISMA] Failed to send client request: " << strerror(errno) << std::endl;
        exit(1);
    }

    int bytes_read = 0;

    int n_bytes = n;
    while (bytes_read < n_bytes) {
        return_value_t = ::read(sock, result + bytes_read, n_bytes - bytes_read);

        if (return_value_t <= 0) {
            std::cerr << "[PRISMA] Error while reading file " << req.filename << std::endl;
            exit(1);
        }

        // Save content in result array
        bytes_read += return_value_t;
    }

    return n;
}


