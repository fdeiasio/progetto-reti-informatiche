#ifndef SERVER_H
#define SERVER_H

#include "common.h"

// Questo  header definisce la struttura server usata sia
// dal server centrale che dal server p2p del client

// Definizione del tipo delle callback chiamate a seguito di un evento
typedef int (*ServerCallback)(void*);

// Struttura server
struct Server {     
    int socket;
    struct sockaddr_in addr;

    fd_set master_set;
    int max_fd;

    ServerCallback client_message_callback;
    ServerCallback stdin_message_callback;
};

// Struttura config usata per inizializzare la struttura server
struct ServerConfig {
    in_port_t port;

    ServerCallback client_message_callback;
    ServerCallback stdin_message_callback;
};

// Alloca il server in una struct dinamica 
// Ritorna il puntatore alla nuova struct
extern struct Server* server_create(struct ServerConfig config);

// Inizializza il server (crea il socket, lo lega alla porta, ecc.)
// Ritorna 0 se l'inizializzazione è andata bene, -1 altrimenti
extern int server_init(struct Server* server);

// Controlla se sono arrivati messaggi al server e chiama le callback corrispondenti
// Ritorna 0 normalmente, se ritorna -1 è stata richiesta la terminazione del server
extern int server_run(struct Server* server);

// Distrugge la struttura allocata e si preoccupa di chiudere i socket aperti
extern void server_shutdown(struct Server* server);

#endif // SERVER_H