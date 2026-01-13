#ifndef UTENTE_UTILS_H
#define UTENTE_UTILS_H

#include "common.h"
#include "client.h"

extern void send_server_hello(int server_fd);

extern void send_server_card_ack(int server_fd);

extern void send_p2p_user_list();

extern void send_pong_server(int server_fd);
#endif // UTENTE_UTILS_H