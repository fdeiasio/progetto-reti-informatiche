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

// Struct globale dello stato dell'utente
extern struct Utente utente;

extern int utente_init(struct Utente* utente, in_port_t port);

extern void utente_update_state(struct Utente* utente, enum UtenteState new_state);

extern int utente_start_p2p(struct Utente* utente, void* p2p_server_function(void*));

extern int utente_start_worker(struct Utente* utente, void* worker_thread_function(void*));

extern void utente_cleanup(struct Utente* utente);

extern in_port_t utente_get_port(struct Utente* utente);


#endif // UTENTE_STATE_H