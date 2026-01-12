#include "../../include/common.h"
#include "../../include/server.h"
#include "../../include/protocol.h"
#include "../../include/thread.h"
#include "../../include/utente_state.h"

#define LOCALHOST "127.0.0.1"

void send_review_request(struct Utente* utente) {
    for (int i = 0; i < utente->num_users; i++) {
        in_port_t peer_port = utente->user_list[i];

        int p2p_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (p2p_socket < 0) {
            continue;
        }
        struct sockaddr_in p2p_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(peer_port),
            .sin_addr.s_addr = inet_addr(LOCALHOST),
        };

        if (connect(p2p_socket, (struct sockaddr *)&p2p_addr, sizeof(p2p_addr)) < 0) {
            fprintf(stderr, "Error: Could not connect to P2P server on port %d.\n", peer_port);
            close(p2p_socket);
            continue;
        }

        uint32_t card_id = htonl(utente->card_id);

        struct Message msg = {
            .type = MSG_REVIEW_CARD,
            .payload_length = sizeof(card_id),
            .payload = &card_id,
        };

        send_message(p2p_socket, &msg);
        close(p2p_socket);

        fprintf(stdout, "Sent REVIEW request to peer on port %d\n", peer_port);
    }
}

int handle_new_peer(struct Server* server, void* args) {
    int fd = *(int*)args;
    
    fprintf(stdout, "New peer connected\n");

    return 0;
}

int handle_peer_message(struct Server* server, int fd, struct Message* msg) {
    switch (msg->type) {
        case MSG_SEND_USER_LIST: {
            
                


            break;
        }

        default:
            break;
    }
    return 0;
}

int handle_peer(struct Server* server, void* args) {
    int fd = *(int*) args;

    uint32_t payload;

    struct Message msg = {
        .payload = &payload,
        .payload_length = sizeof(payload),
    };

    ssize_t bytes_received = receive_message(fd, &msg);
    if (bytes_received <= 0) {
        fprintf(stdout, "Peer disconnected\n");
        return -1;
    }

    return handle_peer_message(server, fd, &msg);
}

void* p2p_server_function(void* arg) {
    struct Utente* utente = (struct Utente*) arg;
    struct ServerConfig config = {
        .port = utente->port,
        .new_client_handler = handle_new_peer,
        .client_handler = handle_peer,
        .stdin_handler = NULL,
    };
    struct Server* peer = server_create(config);
    if (!peer) {
        utente_update_state(utente, STATE_SHUTTING_DOWN);
        pthread_exit(0);
    }

    if (server_init(peer) < 0) {
        server_shutdown(peer);
        utente_update_state(utente, STATE_SHUTTING_DOWN);
        pthread_exit(0);
    }

    utente_update_state(utente, STATE_CONNECTING);
    
    while (utente->state != STATE_DISCONNECTING && server_run(peer) == 0)
        ;

    server_shutdown(peer);

    utente_update_state(utente, STATE_SHUTTING_DOWN);
    pthread_exit(0);
}