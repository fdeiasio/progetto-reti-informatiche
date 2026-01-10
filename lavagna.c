#include "lib/pch.h"
#include "lib/server.h"
#include "lib/messaging.h"

#define SERVER_PORT 5678

void handle_message(Server* server, int fd, Message msg) {
    switch (msg.type) {
        case MSG_HELLO:
            fprintf(stdout, "Received HELLO message from client %d\n", msg.message);
            break;

        default:
            fprintf(stderr, "Error: Unknown message type from client %d\n", fd);
            break;
    }
}

void handle_client(Server* server, void* args) {
    int fd = *(int*) args;

    Message msg;
    ssize_t bytes_received = receive_message(fd, &msg);
    if (bytes_received <= 0) {
        fprintf(stderr, "Client disconnected.\n");

        close(fd);
        server_remove_fd(server, fd);
        return;
    }

    handle_message(server, fd, msg);
}

int main() {
    ServerConfig config = {
        .port = SERVER_PORT,
        .client_handler = handle_client,
        .stdin_handler = NULL,
    };

    Server* lavagna = server_create(config);
    if (server_start(lavagna) < 0) {
        server_shutdown(lavagna);
        exit(1);
    }

    server_run(lavagna);

    server_shutdown(lavagna);
    
    return 0;
}