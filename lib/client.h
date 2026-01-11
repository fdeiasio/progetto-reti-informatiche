#ifndef CLIENT_H
#define CLIENT_H

#include "pch.h"

struct Client;

typedef int (*ClientCallback)(struct Client*);

enum ClientState{
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
};

struct Client {
    in_port_t port;

    enum ClientState state;
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

extern struct Client* client_create(struct ClientConfig config);

extern void client_update_state(struct Client* client, enum ClientState new_state);

extern void client_start_p2p(struct Client* client, void* p2p_server_function(void*));
extern void client_stop_p2p(struct Client* client);

extern void client_connect_to_server(struct Client* client);

extern void client_disconnect_from_server(struct Client* client);
extern int client_listen(struct Client* client);

#endif // CLIENT_H