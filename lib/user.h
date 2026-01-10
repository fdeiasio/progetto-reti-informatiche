#ifndef USER_H
#define USER_H

#include "pch.h"

typedef enum {
    STATE_CONNECTING_P2P,
    STATE_CONNECTING,
    STATE_IDLE,
    STATE_WORKING,
    STATE_WAITING_RUL,
    STATE_SENDING_RUL,
    STATE_WAITING_ACK,
    STATE_DISCONNECTING,
    STATE_SHUTTING_DOWN,

    NUM_STATES,
} UserState;

typedef struct {
    UserState state;
    in_port_t port;

    int server_socket;
    struct sockaddr_in server_addr;

    pthread_t p2p_server_thread;
    pthread_t worker_thread;
} User;

extern User* user_create(in_port_t port);

extern void user_start_p2p(User* user, void* p2p_server_function(void*));

extern void user_connect_to_server(User* user, const char* server_ip, in_port_t server_port);

extern void user_send_server_hello(User* user);

#endif // USER_H