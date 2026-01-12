#include "../../include/common.h"
#include "../../include/utente_state.h"

struct Utente utente;

int utente_init(struct Utente* utente, in_port_t port) {
    if (!utente) {
        return -1;
    }

    utente->port = port;

    utente->state = STATE_STARTING_P2P;
    pthread_mutex_init(&utente->state_mutex, NULL);


    utente->card_id = -1;
    utente->user_list = NULL;
    utente->num_users = 0;

    return 0;
}

void utente_update_state(struct Utente* utente, enum UtenteState new_state) {
    pthread_mutex_lock(&utente->state_mutex);

    utente->state = new_state;

    pthread_mutex_unlock(&utente->state_mutex);
}

int utente_start_p2p(struct Utente* utente, void* p2p_server_function(void*)) {
    if (pthread_create(&utente->p2p_server_thread, NULL, p2p_server_function, utente) != 0) {
        fprintf(stderr, "Error: Could not create P2P server thread.\n");
        return -1;
    }

    return 0;
}

int utente_start_worker(struct Utente* utente, void* worker_thread_function(void*)) {
    if (pthread_create(&utente->worker_thread, NULL, worker_thread_function, utente) != 0) {
        fprintf(stderr, "Error: Could not create worker thread.\n");
        return -1;
    }
    return 0;
}

void utente_cleanup(struct Utente* utente) {
    utente_update_state(utente, STATE_DISCONNECTING);
    pthread_join(utente->p2p_server_thread, NULL);

    pthread_mutex_destroy(&utente->state_mutex);

    if (utente->user_list) {
        free(utente->user_list);
        utente->user_list = NULL;
        utente->num_users = 0;
    }
}


