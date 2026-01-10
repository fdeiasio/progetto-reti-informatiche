#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <sys/select.h>

typedef struct Server Server;
typedef struct ServerConfig ServerConfig;

typedef void (*FDHandler)(Server*, void*);

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

extern Server* server_create(ServerConfig config);

extern int server_start(Server* server);

extern int server_add_fd(Server* server, int fd);

extern void server_run(Server* server);

extern void server_shutdown(Server* server);

#endif // SERVER_H