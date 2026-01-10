#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "p2p_thread.h"

#define MIN_PORT 5679
#define MAX_PORT 65535

#define SERVER_PORT 5678

pthread_t p2p_server_thread;
pthread_t worker_thread;

enum { 
    STATE_CONNECTING,
    STATE_IDLE,
    STATE_WORKING,
    STATE_DISCONNECTING,

    NUM_STATES,
} state = STATE_CONNECTING;

void sighandler(int sig) {
    (void) sig;

    printf("Shutting down client...\n");
    state = STATE_DISCONNECTING;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Setup p2p Client
    int port = atoi(argv[1]);
    if (port < MIN_PORT || port > MAX_PORT) {
        fprintf(stderr, "Error: Invalid port number.\n");
        exit(1);
    }

    pthread_create(&p2p_server_thread, NULL, p2p_server_function, &port);

    /*
    int p2p_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (p2p_socket < 0) {
        fprintf(stderr, "Error: Could not create P2P socket.\n");
        exit(1);
    }

    struct sockaddr_in p2p_addr;
    p2p_addr.sin_family = AF_INET;
    p2p_addr.sin_port = htons(port);
    p2p_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(p2p_socket, (struct sockaddr *)&p2p_addr, sizeof(p2p_addr)) < 0) {
        fprintf(stderr, "Error: Could not bind P2P socket.\n");
        close(p2p_socket);
        exit(1);
    }

    if (listen(p2p_socket, 10) < 0) {
        fprintf(stderr, "Error: Could not listen on P2P socket.\n");
        close(p2p_socket);
        exit(1);
    }

    printf("P2P client listening on port %d\n", port);
    */
    // Connect to Server
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "Error: Could not create socket.\n");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Error: Could not connect to server.\n");
        close(server_fd);
        exit(1);
    }

    fprintf(stdout, "Connected to server\n");

    // Sending initial hello message


    // Set up I/O multiplexing
    fd_set master_set, read_fds;
    FD_ZERO(&master_set);
    
    FD_SET(server_fd, &master_set);
    int max_fd = server_fd;

    FD_SET(STDIN_FILENO, &master_set);
    if (STDIN_FILENO > max_fd) {
        max_fd = STDIN_FILENO;
    }

    // Set up signal handlers
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    state = STATE_IDLE;

    while (state != STATE_DISCONNECTING) {
        read_fds = master_set;
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            fprintf(stderr, "Error: select() failed.\n");
            close(server_fd);
            exit(1);
        }

        if (activity == 0) {
            continue; 
        }

        for (int fd = 0; fd < max_fd; fd++) {
            if (!FD_ISSET(fd, &read_fds)) {
                continue;
            }

            if (fd == server_fd) {

            }
            else if (fd == STDIN_FILENO) {

            }
            else {

            }
        }
    }

    close(server_fd);
    return 0;
}