#include "lib/pch.h"
#include "lib/server.h"
#include "lib/protocol.h"
#include "lib/database.h"

#define SERVER_PORT 5678
#define MAX_USERS 100

static int active = 1;
void signal_handler(int signum) {
    (void)signum;
    
    active = 0;
}

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
    DatabaseConfig db_config = {
        .max_users = MAX_USERS,
    };
    Database* db = database_create(db_config);
    if (!db) {
        exit(1);
    }

    ServerConfig config = {
        .port = SERVER_PORT,
        .client_handler = handle_client,
        .stdin_handler = NULL,
    };
    Server* lavagna = server_create(config);
    if (!lavagna) {
        database_cleanup(db);
        exit(1);
    }

    server_bind_database(lavagna, db);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (server_init(lavagna) < 0) {
        server_shutdown(lavagna);
        database_cleanup(db);
        exit(1);
    }

    while(active && server_run(lavagna) == 0)
        ;

    server_shutdown(lavagna);
    database_cleanup(db);
    exit(0);
}