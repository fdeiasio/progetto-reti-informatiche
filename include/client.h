#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

// Questo header contiene la struttura Client usata dall'applicazione Utente


// Definizione del tipo delle callback chiamate a seguito di un evento
typedef int (*ClientCallback)(void*);

// Struttura client
struct Client {
    int server_socket;
    struct sockaddr_in server_addr;

    fd_set master_set;
    int max_fd;

    ClientCallback server_message_callback;
    ClientCallback stdin_message_callback;
};

// Struttura config usata per inizializzare la struttura client
struct ClientConfig {
    int server_ip;
    in_port_t server_port;

    ClientCallback server_message_callback;
    ClientCallback stdin_message_callback;
};

// Alloca il client in una struct dinamica 
// Ritorna il puntatore alla nuova struct
extern struct Client* client_create(struct ClientConfig config);

// Prova a connettersi al server
// Ritorna 0 se la connessione è avvenuta con successo, -1 altrimenti
extern int client_connect_to_server(struct Client* client);

// Controlla se sono arrivati messaggi al client e chiama le callback corrispondenti
// Ritorna 0 normalmente, se ritorna -1 è stata richiesta la terminazione del client
extern int client_listen(struct Client* client);

// Distrugge la struttura allocata e si preoccupa di chiudere i socket aperti
extern void client_destroy(struct Client* client);

#endif // CLIENT_H