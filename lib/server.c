#include "pch.h"
#include "server.h"

int active = 1;
static void sighandler(int sig) {          
    (void) sig;                      
    printf("Shutting down server...\n");   
    active = 0;               
} 

int server_add_fd(Server* server, int fd) {
    if (fd >= FD_SETSIZE) {
        fprintf(stderr, "Error: File descriptor exceeds FD_SETSIZE.\n");
        return -1;
    }

    FD_SET(fd, &server->master_set);
    if (fd > server->max_fd) {
        server->max_fd = fd;
    }

    return 0;
}

int server_remove_fd(Server* server, int fd) {
    if (fd >= FD_SETSIZE) {
        fprintf(stderr, "Error: File descriptor exceeds FD_SETSIZE.\n");
        return -1;
    }

    FD_CLR(fd, &server->master_set);

    if (fd == server->max_fd) {
        while (server->max_fd >= 0 && !FD_ISSET(server->max_fd, &server->master_set)) {
            server->max_fd--;
        }
    }

    return 0;
}

void server_handle_new_client(Server* server, void* args) {
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

Server* server_create(ServerConfig config) {
    Server* server = (Server*)malloc(sizeof(Server));
    if (!server) {
        fprintf(stderr, "Error: Could not allocate memory for server.\n");
        exit(1);
    }

    server->socket_fd = -1;
    server->addr.sin_family = AF_INET;
    server->addr.sin_port = htons(config.port);
    server->addr.sin_addr.s_addr = INADDR_ANY;

    FD_ZERO(&server->master_set);
    FD_ZERO(&server->read_fds);
    server->max_fd = 0;

    server->new_client_handler = config.new_client_handler 
        ? config.new_client_handler 
        : server_handle_new_client;

    server->stdin_handler = config.stdin_handler;
    server->client_handler = config.client_handler;

    return server;
}

int server_start(Server* server) {
    server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket_fd < 0) {
        fprintf(stderr, "Error: Could not create socket.\n");
        return -1;
    }

    if (bind(server->socket_fd, (struct sockaddr *)&server->addr, sizeof(server->addr)) < 0) {
        fprintf(stderr, "Error: Could not bind socket.\n");
        close(server->socket_fd);
        return -1;
    }

    if (listen(server->socket_fd, 10) < 0) {
        fprintf(stderr, "Error: Could not listen on socket.\n");
        close(server->socket_fd);
        return -1;
    }

    if (server->new_client_handler) {
        server_add_fd(server, server->socket_fd);
    }

    if (server->stdin_handler) {
        server_add_fd(server, STDIN_FILENO);
    }

    printf("Server started on port %d\n", ntohs(server->addr.sin_port));

    return 0;
}

void server_run(Server* server) {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    while (active) {
        server->read_fds = server->master_set;

        struct timeval timeout = { 
            .tv_sec = 1, 
            .tv_usec = 0 
        };

        int activity = select(server->max_fd + 1, &server->read_fds, NULL, NULL, &timeout);
        if (activity < 0) {
            fprintf(stderr, "Error: select() failed.\n");
            break;
        }

        if (activity == 0) {
            continue; 
        }

        for (int fd = 0; fd <= server->max_fd; fd++) {
            if (!FD_ISSET(fd, &server->read_fds)) {
                continue;
            }

            if (fd == server->socket_fd) {
                server->new_client_handler(server, NULL);
            }
            else if (fd == STDIN_FILENO) {
                server->stdin_handler(server, NULL);
            }
            else {
                int arg = fd;
                server->client_handler(server, &arg);
            }
        }
    }
}

void server_shutdown(Server* server) {
    for (int fd = 0; fd <= server->max_fd; fd++) {
        if (FD_ISSET(fd, &server->master_set)) {
            close(fd);
        }
    }
    free(server);
}
