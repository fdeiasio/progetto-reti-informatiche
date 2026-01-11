#ifndef CLIENT_H
#define CLIENT_H

#include "pch.h"

typedef struct Client Client;
typedef struct ClientConfig ClientConfig;

typedef int (*ClientCallback)(Client*);

typedef enum {
    STATE_STARTING_P2P,
    STATE_CONNECTING,
    STATE_IDLE,
    STATE_WORKING,
    STATE_WAITING_RUL,
    STATE_SENDING_RUL,
    STATE_WAITING_ACK,
    STATE_DISCONNECTING,
    STATE_SHUTTING_DOWN,

    NUM_STATES,
} ClientState;

struct Client {
    in_port_t port;

    ClientState state;
    pthread_mutex_t state_mutex;

    int server_socket;
    struct sockaddr_in server_addr;

    fd_set master_set;
    fd_set read_fds;
    int max_fd;

    ClientCallback server_handler;
    ClientCallback stdin_handler;

    pthread_t p2p_server_thread;
    pthread_t worker_thread;
};

struct ClientConfig {
    in_port_t client_port;

    int server_ip;
    in_port_t server_port;

    ClientCallback server_handler;
    ClientCallback stdin_handler;
};

extern Client* client_create(ClientConfig config);

extern void client_update_state(Client* client, ClientState new_state);

extern void client_start_p2p(Client* client, void* p2p_server_function(void*));

extern void client_connect_to_server(Client* client);

extern int client_listen(Client* client);

#endif // CLIENT_H