#ifndef LAVAGNA_UTILS_H
#define LAVAGNA_UTILS_H

#include "common.h"

// Questo header contiene funzioni di utilità per l'applicazione lavagna

#define WORKING_TIMEOUT_SECONDS 4
#define PING_TIMEOUT_SECONDS 2
#define TIMEOUT_CHECK_PERIOD 1

// Inviano il messaggio specificato al socket specificato
// Ritornano 0 in caso di successo, -1 in caso di errore
extern int send_user_list(int socket);
extern int send_user_ping(int socket);
extern int send_user_quit(int socket);

// Assegnano una card ad un utente idle, se possibile
// Ritorna -1 se non ha assegnato
extern int assign_card();

// Procedura per disconnettere un utente
extern void disconnect_user(in_port_t user_port);

// Procedura per controllare i timeout
extern void check_timeout();

#endif