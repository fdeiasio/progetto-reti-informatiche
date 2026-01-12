#include "../../include/common.h"
#include "../../include/utente_state.h"

struct Utente utente;

void utente_init(in_port_t port) {
    utente.port = port;
    utente.state = STATE_STARTING_P2P;
    pthread_mutex_init(&utente.state_mutex, NULL);

    utente.card_id = -1;
    utente.user_list = NULL;
    utente.num_users = 0;
}

enum UtenteState utente_get_state() {
    enum UtenteState state;
    
    pthread_mutex_lock(&utente.state_mutex);
    state = utente.state;
    pthread_mutex_unlock(&utente.state_mutex);

    return state;
}

void utente_update_state(enum UtenteState new_state) {
    pthread_mutex_lock(&utente.state_mutex);

    utente.state = new_state;

    pthread_mutex_unlock(&utente.state_mutex);
}

in_port_t utente_get_port() {
    return utente.port;
}

int utente_start_p2p(void* p2p_server_function(void*)) {
    if (pthread_create(&utente.p2p_server_thread, NULL, p2p_server_function, &utente) != 0) {
        fprintf(stderr, "Error: Could not create P2P server thread.\n");
        return -1;
    }

    return 0;
}

int utente_start_worker(void* worker_thread_function(void*)) {
    if (pthread_create(&utente.worker_thread, NULL, worker_thread_function, &utente) != 0) {
        fprintf(stderr, "Error: Could not create worker thread.\n");
        return -1;
    }
    return 0;
}

void utente_cleanup() {
    pthread_mutex_destroy(&utente.state_mutex);

    if (utente.user_list) {
        free(utente.user_list);
        utente.user_list = NULL;
        utente.num_users = 0;
    }
}


int utente_allocate_user_list(in_port_t** user_list, int num_users) {
    if (utente.user_list) {
        free(utente.user_list);
    }
    utente.user_list = malloc(num_users * sizeof(in_port_t));
    if (!utente.user_list) {
        fprintf(stderr, "Error: Could not allocate memory for user list.\n");
        return -1;
    }

    memcpy(utente.user_list, *user_list, num_users * sizeof(in_port_t));
    utente.num_users = num_users;

    return 0;
}

void utente_show_user_list() {
    fprintf(stdout, "==== Current Users ====\n");
    if (utente.num_users == 0) {
        fprintf(stdout, "(no users)\n");
    } else {
        for (int i = 0; i < utente.num_users; i++) {
            fprintf(stdout, "User %d: Port=%d\n", i + 1, utente.user_list[i]);
        }
    }
    fprintf(stdout, "=======================\n");
}

void utente_set_card_id(int card_id) {
    utente.card_id = card_id;
}

int utente_get_card_id() {
    return utente.card_id;
}

void utente_wait_for_p2p_shutdown() {
    utente_update_state(STATE_DISCONNECTING);
    pthread_join(utente.p2p_server_thread, NULL);
}

void utente_wait_for_worker_shutdown() {
    pthread_join(utente.worker_thread, NULL);
}

int utente_get_users(in_port_t** user_ports) {
    if (utente.num_users == 0 || !utente.user_list) {
        *user_ports = NULL;
        return 0;
    }

    *user_ports = malloc(utente.num_users * sizeof(in_port_t));
    if (!*user_ports) {
        fprintf(stderr, "Error: Could not allocate memory for user ports.\n");
        return -1;
    }

    memcpy(*user_ports, utente.user_list, utente.num_users * sizeof(in_port_t));
    return utente.num_users;
}