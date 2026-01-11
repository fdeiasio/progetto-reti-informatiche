#ifndef SERVER_H
#define SERVER_H

#include "pch.h"

struct Server;

typedef int (*ServerCallback)(struct Server*, void*);

struct Server {     
    int socket_fd;
    struct sockaddr_in addr;

    fd_set master_set;
    fd_set read_fds;
    int max_fd;

    ServerCallback new_client_handler;
    ServerCallback client_handler;
    ServerCallback stdin_handler;
};

struct ServerConfig {
    in_port_t port;

    ServerCallback new_client_handler;
    ServerCallback client_handler;
    ServerCallback stdin_handler;
};

int server_add_fd(struct Server* server, int fd);

int server_remove_fd(struct Server* server, int fd);

extern struct Server* server_create(struct ServerConfig config);

extern int server_init(struct Server* server);

extern int server_run(struct Server* server);

extern void server_shutdown(struct Server* server);

#endif // SERVER_H