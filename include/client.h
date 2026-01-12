#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"
/**
 * Questo header definisce la struttura e le funzioni di un client TCP.
 * Include anche la parte P2P del client.
 */
struct Client;

typedef int (*ClientCallback)(struct Client*);

/**
 * Struttura che rappresenta il client.
 * 
 * server_socket: File descriptor del socket del server a cui il client è connesso
 * server_addr: Indirizzo del server
 * 
 * master_set: Set di file descriptor monitorati dal client
 * read_fds: Set di file descriptor pronti per la lettura
 * max_fd: Massimo file descriptor attualmente monitorato
 * 
 * server_handler: Callback chiamata quando il server invia un messaggio
 * stdin_handler: Callback chiamata quando c'è input da stdin
 */
struct Client {
    int server_socket;
    struct sockaddr_in server_addr;

    fd_set master_set;
    fd_set read_fds;
    int max_fd;

    ClientCallback server_handler;
    ClientCallback stdin_handler;
};

/**
 * Struttura di configurazione usata per creare un client.
 * 
 * server_ip: Indirizzo IP del server a cui connettersi
 * server_port: Porta del server a cui connettersi
 * 
 * callback: corrispondenti alle callback descritte nella struct Client
 */
struct ClientConfig {
    int server_ip;
    in_port_t server_port;

    ClientCallback server_handler;
    ClientCallback stdin_handler;
};

/**
 * Funzioni per creare, connettere e distruggere un client.
 */
extern struct Client* client_create(struct ClientConfig config);

extern int client_connect_to_server(struct Client* client);

extern int client_listen(struct Client* client);

extern void client_destroy(struct Client* client);

#endif // CLIENT_H