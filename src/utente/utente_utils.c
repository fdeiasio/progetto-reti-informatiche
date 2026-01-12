#include "../../include/common.h"
#include "../../include/utente_utils.h"
#include "../../include/client.h"
#include "../../include/utente_state.h"
#include "../../include/protocol.h"

void send_server_hello(struct Client* client) {
    in_port_t port_network = htons(utente.port);
    struct Message msg = {
        .type = MSG_HELLO,
        .payload_length = sizeof(port_network),
        .payload = &port_network,
    };

    if (send_message(client->server_socket, &msg) < 0) {
        fprintf(stderr, "Error: Could not send HELLO message to server.\n");
        return;
    } 

    fprintf(stdout, "Sent HELLO message to server\n");
}

void send_server_card_ack(struct Client* client) {
    struct Message msg = {
        .type = MSG_ACK_CARD,
        .payload_length = 0,
        .payload = NULL,
    };

    if (send_message(client->server_socket, &msg) < 0) {
        fprintf(stderr, "Error: Could not send ACK_CARD message to server.\n");
        return;
    } 

    fprintf(stdout, "Sent ACK_CARD message to server\n");
}

void send_p2p_user_list(struct Client* client) {
    int p2p_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (p2p_socket < 0) {
        fprintf(stderr, "Error: Could not create P2P socket.\n");
        return;
    }
    struct sockaddr_in p2p_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(utente.port),
        .sin_addr.s_addr = inet_addr(LOCALHOST),
    };

    if (connect(p2p_socket, (struct sockaddr *)&p2p_addr, sizeof(p2p_addr)) < 0) {
        fprintf(stderr, "Error: Could not connect to P2P server.\n");
        close(p2p_socket);
        return;
    }

    struct Message msg = {
        .type = MSG_SEND_USER_LIST,
        .payload_length = 0,
        .payload = NULL,
    };

    if (send_message(p2p_socket, &msg) < 0) {
        fprintf(stderr, "Error: Could not send user list to P2P server.\n");
    }

    close(p2p_socket);
}

