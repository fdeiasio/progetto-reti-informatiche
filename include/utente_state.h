#ifndef UTENTE_STATE_H
#define UTENTE_STATE_H

#include "common.h"

enum UtenteState{
    STATE_STARTING_P2P,
    STATE_CONNECTING,
    STATE_IDLE,

    STATE_WORKING,

    STATE_REQUESTING_UL,
    STATE_WAITING_UL,
    STATE_WAITING_ACK,
    STATE_DONE_WORK,

    STATE_DISCONNECTING,
    STATE_SHUTTING_DOWN,

    NUM_STATES,
};

struct Utente {
    in_port_t port;

    enum UtenteState state;
    pthread_mutex_t state_mutex;

    int card_id;
    in_port_t* user_list;
    int num_users;

    pthread_t p2p_server_thread;
    pthread_t worker_thread;
};

extern int utente_init(in_port_t port);

extern enum UtenteState utente_get_state();

extern void utente_update_state(enum UtenteState new_state);

extern int utente_start_p2p(void* p2p_server_function(void*));

extern int utente_start_worker(void* worker_thread_function(void*));

extern void utente_cleanup();

extern in_port_t utente_get_port();

extern int utente_allocate_user_list(in_port_t** user_list, int num_users);

extern int utente_get_users(in_port_t** user_ports);

extern void utente_show_user_list();

extern int utente_get_card_id();

extern void utente_set_card_id(int card_id);

extern void utente_wait_for_p2p_shutdown();

extern void utente_wait_for_worker_shutdown();


#endif // UTENTE_STATE_H