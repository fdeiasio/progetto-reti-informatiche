#ifndef UTENTE_STATE_H
#define UTENTE_STATE_H

#include "common.h"

// Questo header serve a tenere lo stato globale dell'utente


enum UtenteState{
    STATE_STARTING_P2P,     // Stato iniziale  
    STATE_CONNECTING,       // Connessione al server centrale
    STATE_IDLE,             // Default

    STATE_WORKING,          // Il worker thread sta lavorando
    STATE_REQUESTING_UL,    // Il worker thread ha terminato, chiedo la user list
    STATE_WAITING_UL,       // Aspetto la user list, se viene mandata la giro al server p2p
    STATE_WAITING_ACK,      // Aspetto che il server riceva un ack dai peer
    STATE_DONE_WORK,        // Ho finito di lavorare, mando card done al server

    STATE_DISCONNECTING,    // Notifico il server p2p che mi sto disconnettendo
    STATE_SHUTTING_DOWN,    // Il server p2p invia al client la notifica che si sta spegnendo


    NUM_STATES,
};

// struct che tiene lo stato globale dell'utente
struct Utente {
    in_port_t port;
    enum UtenteState state;
    
    pthread_mutex_t mutex;

    int card_id;
    in_port_t* user_list;
    int num_users;

    pthread_t p2p_server_thread;
    pthread_t worker_thread;
};

// Inizializza lo stato globale dell'utente
extern void utente_init(in_port_t port);

// Ritorna la porta dell'utente
extern in_port_t utente_get_port();

// Ritorna lo stato corrente dell'utente protetto da mutex
extern enum UtenteState utente_get_state();

// Aggiorna lo stato corrente dell'utente protetto da mutex
extern void utente_update_state(enum UtenteState new_state);

// Imposta l'ID della card assegnata all'utente
extern void utente_set_card_id(int card_id);

// Ritorna l'ID della card assegnata all'utente
extern int utente_get_card_id();

// Copia user_list nella lista interna dell'utente
extern int utente_allocate_user_list(in_port_t** user_list, int num_users);

// Allora la lista degli utenti connessi e torna il numero di utenti
extern int utente_get_users(in_port_t** user_ports);

// Inizializza e avvia il server P2P in un thread separato
extern int utente_start_p2p(void* p2p_server_function(void*));

// Attende la terminazione del server P2P
extern void utente_wait_for_p2p_shutdown();

// Inizializza e avvia il worker thread in un thread separato
extern int utente_start_worker(void* worker_thread_function(void*));

// Attende la terminazione del worker thread
extern void utente_wait_for_worker_shutdown();

// Distrugge le risorse allocate nello stato globale dell'utente
extern void utente_cleanup();


#endif // UTENTE_STATE_H