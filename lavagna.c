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

int new_client_handler(Server* server, void* args) {
    int fd = *(int*)args;
    
    fprintf(stdout, "New client connected\n");

    return 0;
}

int handle_message(Server* server, int fd, Message msg) {
    switch (msg.type) {
        case MSG_HELLO:
            fprintf(stdout, "Received HELLO message from client %d\n", msg.message);

            User new_user = {
                .fd = fd,
                .port = msg.message,
            };

            database_add_user(&new_user);
            database_print_users();

            break;

        default:
            fprintf(stderr, "Error: Unknown message type from client %d\n", fd);
            break;
    }

    return 0;
}

int handle_client(Server* server, void* args) {
    int fd = *(int*) args;

    Message msg;
    ssize_t bytes_received = receive_message(fd, &msg);
    if (bytes_received <= 0) {
        int port = database_get_port_from_fd(fd);

        if (port != -1) {
            fprintf(stdout, "Client %d disconnected\n", port);
            database_remove_user(port);
        } 
        else {
            fprintf(stdout, "Unknown client disconnected\n");
        }
        
        database_print_users();
        return -1;
    }

    return handle_message(server, fd, msg);
}

int main() {
    DatabaseConfig db_config = {
        .max_users = MAX_USERS,
    }; 
    if (database_init(db_config) < 0) {
        exit(1);
    }

    ServerConfig config = {
        .port = SERVER_PORT,
        .new_client_handler = new_client_handler,
        .client_handler = handle_client,
        .stdin_handler = NULL,
    };
    Server* lavagna = server_create(config);
    if (!lavagna) {
        database_cleanup();
        exit(1);
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (server_init(lavagna) < 0) {
        server_shutdown(lavagna);
        database_cleanup();
        exit(1);
    }

    while(active && server_run(lavagna) == 0)
        ;

    server_shutdown(lavagna);
    database_cleanup();
    exit(0);
}