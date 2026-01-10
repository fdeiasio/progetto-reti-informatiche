#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.h"
#include "common.h"

#define SERVER_PORT 5678

void handle_new_client(Server* server, void* args) {
    (void) args;

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int new_fd = accept(server->socket_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (new_fd < 0) {
        fprintf(stderr, "Error: Failed to accept connection.\n");
        return;
    }

    if (server_add_fd(server, new_fd) < 0) {
        close(new_fd);
        return;
    }
    
    fprintf(stdout, "New client connected\n");
}

int main() {
    ServerConfig config = {
        .port = SERVER_PORT,
        .new_client_handler = handle_new_client,
        .client_handler = NULL,
        .stdin_handler = NULL,
    };

    Server* lavagna = server_create(config);
    if (server_start(lavagna) < 0) {
        free(lavagna);
        exit(1);
    }

    server_run(lavagna);

    server_shutdown(lavagna);
    
    return 0;
}