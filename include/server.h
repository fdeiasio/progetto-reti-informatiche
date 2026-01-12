#ifndef SERVER_H
#define SERVER_H

#include "common.h"

/**
 * Questo header definisce la struttura e le funzioni di un server TCP.
 * È usata sia per implementare il server centrale, che la parte server
 * del client P2P. 
 */

struct Server;

/* *
 * Definizione del tipo delle callback usate dal server.
 * Queste funzioni vengono chiamate in automatico dal server 
 * quando si verificano certi eventi. 
 * Prende in input il server e un puntatore a argomenti 
 * opzionali (di solito l'fd del client).
 */
typedef int (*ServerCallback)(struct Server*, void*);

/**
 * Struttura che rappresenta il server.
 * 
 * socket_fd: File descriptor del socket del server
 * addr: Indirizzo del server
 * 
 * master_set: Set di file descriptor monitorati dal server
 * read_fds: Set di file descriptor pronti per la lettura
 * max_fd: Massimo file descriptor attualmente monitorato
 * 
 * new_client_handler: Callback chiamata quando un nuovo client si connette
 * client_handler: Callback chiamata quando un client invia un messaggio
 * stdin_handler: Callback chiamata quando c'è input da stdin (può essere NULL se il server non lo usa)
 */
struct Server {     
    int socket_fd;
    struct sockaddr_in addr;

    fd_set master_set;
    fd_set read_fds;
    int max_fd;

    ServerCallback new_client_handler;
    ServerCallback client_handler;
    ServerCallback stdin_handler;
};

/**
 * Struttura di configurazione usata per creare un server.
 * 
 * port: Porta su cui il server ascolta
 * callback: corrispondenti alle callback descritte nella struct Server
 */
struct ServerConfig {
    in_port_t port;

    ServerCallback new_client_handler;
    ServerCallback client_handler;
    ServerCallback stdin_handler;
};

/**
 * Crea un nuovo server con la configurazione specificata.
 * Ritorna un puntatore al server creato, o NULL in caso di errore.
 */
extern struct Server* server_create(struct ServerConfig config);

/* *
 * Inizializza il server: crea il socket, lo lega alla porta
 * e inizia ad ascoltare le connessioni in arrivo.
 * Ritorna 0 in caso di successo, -1 in caso di errore.
 */
extern int server_init(struct Server* server);

/* *
 * Esegue un ciclo di ascolto del server.
 * Ritorna 0 in condizoni di successo, -1 in caso di errore.
 * 
 * Questa funzione deve essere chiamata ripetutamente in un loop
 * per mantenere il server attivo. Se ritorna -1, il server deve essere
 * chiuso chiamando server_shutdown().
 */
extern int server_run(struct Server* server);

/* *
 * Chiude il server, liberando tutte le risorse allocate.
 */
extern void server_shutdown(struct Server* server);

#endif // SERVER_H