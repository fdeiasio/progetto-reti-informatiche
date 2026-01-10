#ifndef CLIENT_H
#define CLIENT_H

#include "pch.h"

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

typedef struct {
    in_port_t port;

    ClientState state;
    pthread_mutex_t state_mutex;

    int server_socket;
    struct sockaddr_in server_addr;

    pthread_t p2p_server_thread;
    pthread_t worker_thread;
} Client;

extern Client* client_create(in_port_t port);

extern void client_update_state(Client* client, ClientState new_state);

extern void client_start_p2p(Client* client, void* p2p_server_function(void*));

extern void client_connect_to_server(Client* client, const char* server_ip, in_port_t server_port);

extern void client_send_server_hello(Client* client);

#endif // CLIENT_H