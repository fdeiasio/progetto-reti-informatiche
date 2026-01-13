#include "../../include/common.h"
#include "../../include/server.h"

// Funzioni statiche di aiuto

// Aggiunge un file descriptor al set di monitoraggio del server
static int server_add_fd(struct Server* server, int fd) {
    if (fd >= FD_SETSIZE) {
        return -1;
    }

    FD_SET(fd, &server->master_set);
    if (fd > server->max_fd) {
        server->max_fd = fd;
    }

    return 0;
}

// Lo rimuove
static int server_remove_fd(struct Server* server, int fd) {
    if (fd >= FD_SETSIZE) {
        return -1;
    }

    close(fd);
    FD_CLR(fd, &server->master_set);

    if (fd == server->max_fd) {
        while (server->max_fd >= 0 && !FD_ISSET(server->max_fd, &server->master_set)) {
            server->max_fd--;
        }
    }

    return 0;
}

struct Server* server_create(struct ServerConfig config) {
    struct Server* server = (struct Server*)malloc(sizeof(struct Server));
    if (!server) {
        return NULL;
    }

    server->socket = -1;
    server->addr.sin_family = AF_INET;
    server->addr.sin_port = htons(config.port);
    server->addr.sin_addr.s_addr = INADDR_ANY;

    FD_ZERO(&server->master_set);
    server->max_fd = 0;

    server->stdin_message_callback = config.stdin_message_callback;
    server->client_message_callback = config.client_message_callback;

    return server;
}

int server_init(struct Server* server) {
    server->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket < 0) {
        return -1;
    }

    if (setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        close(server->socket);
        server->socket = -1;
        return -1;
    }

    if (bind(server->socket, (struct sockaddr *)&server->addr, sizeof(server->addr)) < 0) {
        close(server->socket);
        server->socket = -1;
        return -1;
    }

    if (listen(server->socket, 10) < 0) {
        close(server->socket);
        server->socket = -1;
        return -1;
    }

    // Ignoro la signal SIGPIPE per evitare che il server termini improvvisamente
    signal(SIGPIPE, SIG_IGN);
    
    // Aggiungo il socket del server al set di monitoraggio per gestire le nuove connessioni
    server_add_fd(server, server->socket);

    // Aggiungo lo standard input al set di monitoraggio se è definita una callback
    if (server->stdin_message_callback) {
        server_add_fd(server, STDIN_FILENO);
    }

    return 0;
}

int server_run(struct Server* server) {
    fd_set read_fds = server->master_set;

    struct timeval timeout = { 
        .tv_sec = 0, 
        .tv_usec = 100000 
    };

    int activity = select(server->max_fd + 1, &read_fds, NULL, NULL, &timeout);
    if (activity < 0) {
        return -1;
    }

    // Se non c'è attività, ritorno 0 per continuare il loop
    if (activity == 0) {
        return 0; 
    }

    for (int fd = 0; fd <= server->max_fd; fd++) {
        if (!FD_ISSET(fd, &read_fds)) {
            continue;
        }

        if (fd == server->socket) {
            // Gestisco un nuovo client in arrivo

            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);

            int new_fd = accept(server->socket, (struct sockaddr *)&client_addr, &addr_len);
            if (new_fd < 0) {
                continue;
            }

            if (server_add_fd(server, new_fd) < 0) {
                close(new_fd);
                continue;
            }

        }
        else if (fd == STDIN_FILENO) {
            // Gestisco l'input da stdin

            if (server->stdin_message_callback && server->stdin_message_callback(NULL) < 0) {
                // Chiudo il server se stdin_message_callback ritorna -1
                return -1;
            }
        }
        else {
            // Gestisco un messaggio da un client esistente
            
            int arg = fd;
            if (server->client_message_callback && server->client_message_callback(&arg) < 0) {
                server_remove_fd(server, fd);
                continue;
            }
        }
    }

    return 0;
}

void server_shutdown(struct Server* server) {
    if (!server) {
        return;
    }

    for (int fd = 0; fd <= server->max_fd; fd++) {
        if (FD_ISSET(fd, &server->master_set) && fd != STDIN_FILENO) {
            close(fd);
        }
    }

    free(server);
}