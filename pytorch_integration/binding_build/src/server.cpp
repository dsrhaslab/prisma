#include <netinet/in.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <iostream>

#include "server.h"
#include "client.h"

void Server::create_connection(){
    // Creating socket file descriptor
    const char* socket_name = "prisma-socket";
    unlink (socket_name);

    if ((server_fd = socket (AF_UNIX, SOCK_STREAM, 0)) == 0) {
        std::cerr << "[PRISMA] Socket creation error: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        std::cout << "[PRISMA] Server socket created." << std::endl;
    }

    unix_socket.sun_family = AF_UNIX;
    strncpy (unix_socket.sun_path, socket_name, sizeof(unix_socket.sun_path) - 1);

    if (bind (server_fd, (struct sockaddr*) &unix_socket, sizeof(unix_socket)) < 0) {
        std::cerr << "[PRISMA] Bind error: " << strerror(errno) << std::endl;
        exit(1);
    }

    if (listen (server_fd, 50) < 0) {
        std::cerr << "[PRISMA] Listen error: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        std::cout << "[PRISMA] Server listening..." << std::endl;
    }

    addr_len = sizeof (unix_socket);
}

int Server::accept(){
    int new_socket_t = -1;
    // Accept new PRISMA client connection
    new_socket_t = ::accept(server_fd, (struct sockaddr*) &unix_socket, (socklen_t *) &addr_len);

    // Verify socket value
    if (new_socket_t == -1) {
        std::cerr << "[PRISMA] Failed to connect with PRIMSA client." << std::endl;
    } else {
        std::cout << "[PRISMA] New PRIMSA client connection established." << std::endl;
    }

    return new_socket_t;
}

void Server::handle_request(int sock) {
    int return_value_t;

    // Read request
    Client::request req{};
    int res = ::read(sock, &req, sizeof(req));
    if (res != sizeof(struct Client::request)) {
        std::cerr << "[PRISMA] Failed to receive client request: " << strerror(errno) << std::endl;
        close(sock);
        return;
    }

    while(res > 0) {
        // Read data from buffer
        char* result_buffer = new char[req.n_bytes];
        bzero(result_buffer, req.n_bytes);
        int r = prisma.read(req.filename, result_buffer, req.n_bytes, req.offset);

        if (r == 0) {
            std::cerr << "[PRISMA] Error while reading file " << req.filename << std::endl;
        }
        return_value_t = ::write(sock, result_buffer, r);
        if (return_value_t != r) {
            std::cerr << "[PRISMA] Failed to send server response: " << strerror(errno) << std::endl;
            exit(1);
        }

        res = ::read(sock, &req, sizeof(struct Client::request));
        if (res != sizeof(struct Client::request)) {
            std::cerr << "[PRISMA] Failed to receive client request: " << strerror(errno) << std::endl;
            close(sock);
            return;
        }

        // Free memory
        delete[] result_buffer;
    }
}

void Server::run() {
    // Start Prisma
    prisma = Prisma();

    // Prepare UNIX connection
    create_connection();

    while(1) {
        std::cout << "[PRISMA] Waiting for client connection ..." << std::endl;
        // Accept PRISMA client connection
        int socket_t = accept();

        if (socket_t != -1) {
            // Start handler thread for this connection
            std::thread handler_thread(&Server::handle_request, this, socket_t);
            handler_thread.detach();

            std::cout << "[PRISMA] Connecting and going for the next one ..." << std::endl;
        }
    }
}
