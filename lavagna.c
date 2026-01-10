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

#define SERVER_PORT 5678

int main() {
    ServerConfig config = {
        .port = SERVER_PORT,
        .new_client_handler = NULL,
        .client_handler = NULL,
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