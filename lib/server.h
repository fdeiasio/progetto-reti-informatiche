#ifndef SERVER_H
#define SERVER_H

#include "pch.h"

typedef struct Server Server;
typedef struct ServerConfig ServerConfig;

typedef int (*FDHandler)(Server*, void*);

struct Server {     
    int socket_fd;
    struct sockaddr_in addr;

    fd_set master_set;
    fd_set read_fds;
    int max_fd;

    FDHandler new_client_handler;
    FDHandler client_handler;
    FDHandler stdin_handler;
};

struct ServerConfig {
    in_port_t port;

    FDHandler new_client_handler;
    FDHandler client_handler;
    FDHandler stdin_handler;
};

int server_add_fd(Server* server, int fd);

int server_remove_fd(Server* server, int fd);

extern Server* server_create(ServerConfig config);

extern int server_init(Server* server);

extern int server_run(Server* server);

extern void server_shutdown(Server* server);

#endif // SERVER_H