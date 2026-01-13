#include "../../include/common.h"
#include "../../include/server.h"
#include "../../include/protocol.h"
#include "../../include/utente_state.h"
#include "../../include/p2p_utils.h"

int p2p_broadcast_review_request() {
    fprintf(stdout, "P2P: Broadcasting review request to peers...\n");

    in_port_t* user_ports;
    int num_users = utente_get_users(&user_ports);
    if (num_users <= 0 || !user_ports) {
        fprintf(stdout, "P2P: No peers available to send review request.\n");
        return 0;
    }

    for (int i = 0; i < num_users; i++) {
        if (user_ports[i] == utente_get_port()) {
            continue; 
        }

        int peer_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (peer_socket < 0) {
            fprintf(stderr, "P2P: Error creating socket for peer on port %d\n", user_ports[i]);
            continue;
        }

        struct sockaddr_in peer_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(user_ports[i]),
            .sin_addr.s_addr = inet_addr(LOCALHOST),
        };

        if (connect(peer_socket, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
            fprintf(stderr, "P2P: Error connecting to peer on port %d\n", user_ports[i]);
            close(peer_socket);
            continue;
        }

        in_port_t payload = htons(utente_get_port());

        struct Message msg = {
            .type = MSG_REVIEW_CARD,
            .payload_length = sizeof(in_port_t),
            .payload = &payload,
        };

        if (send_message(peer_socket, &msg) < 0) {
            fprintf(stderr, "P2P: Error sending review request to peer on port %d\n", user_ports[i]);
        } 

        close(peer_socket);
    }

    free(user_ports);

    return num_users - 1;
}

void p2p_send_done_review(int peer_port) {
    int peer_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (peer_socket < 0) {
        fprintf(stderr, "P2P: Error creating socket for peer on port %d\n", peer_port);
        return;
    }

    struct sockaddr_in peer_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(peer_port),
        .sin_addr.s_addr = inet_addr(LOCALHOST),
    };

    if (connect(peer_socket, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
        fprintf(stderr, "P2P: Error connecting to peer on port %d\n", peer_port);
        close(peer_socket);
        return;
    }

    in_port_t payload = htons(utente_get_port());

    struct Message msg = {
        .type = MSG_DONE_REVIEW,
        .payload_length = sizeof(in_port_t),
        .payload = &payload,
    };

    if (send_message(peer_socket, &msg) < 0) {
        fprintf(stderr, "P2P: Error sending done review to peer on port %d\n", peer_port);
    } 

    close(peer_socket);
}