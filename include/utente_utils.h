#ifndef UTENTE_UTILS_H
#define UTENTE_UTILS_H

#include "common.h"
#include "client.h"

extern void send_server_hello(struct Client* client);

extern void send_server_card_ack(struct Client* client);

extern void send_p2p_user_list(struct Client* client);

#endif // UTENTE_UTILS_H