#ifndef UTENTE_UTILS_H
#define UTENTE_UTILS_H

#include "common.h"
#include "client.h"

extern int send_server_hello(int server_fd);

extern int send_server_card_ack(int server_fd);

extern int send_server_pong(int server_fd);

extern int send_p2p_user_list();

#endif // UTENTE_UTILS_H