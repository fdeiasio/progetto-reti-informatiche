#include "../../include/common.h"
#include "../../include/server.h"
#include "../../include/protocol.h"
#include "../../include/utente_state.h"
#include "../../include/p2p_utils.h"

static int remaining_acks = 0;

int handle_peer_message(struct Message* msg) {
    switch (msg->type) {
        case MSG_SEND_USER_LIST: {
            remaining_acks = p2p_broadcast_review_request();
            
            if (remaining_acks == 0) {
                utente_update_state(STATE_DONE_WORK);
            }
            break;
        }

        case MSG_REVIEW_CARD: {
            in_port_t peer_port = ntohs(*(in_port_t*)msg->payload);
            fprintf(stdout, "P2P: Received review request from peer on port %d.\n", peer_port);

            p2p_send_done_review(peer_port);

            break; 
        }

        case MSG_DONE_REVIEW: {
            in_port_t peer_port = ntohs(*(in_port_t*)msg->payload);
            fprintf(stdout, "P2P: Received DONE_REVIEW from peer on port %d.\n", peer_port);

            remaining_acks--;

            if (remaining_acks == 0) {
                utente_update_state(STATE_DONE_WORK);
            }
            break;
        }

        default:
            fprintf(stderr, "P2P: Unknown message type from peer.\n");
            break;
    }

    // Default to close the connection
    return -1;
}

int handle_peer(void* args) {
    int fd = *(int*) args;

    // Per i peer non sono definiti messaggi con payload più grandi di 4 byte
    uint32_t payload;

    struct Message msg = {
        .payload = &payload,
        .payload_length = sizeof(payload),
    };

    ssize_t bytes_received = receive_message(fd, &msg);
    if (bytes_received <= 0) {
        return -1;
    }

    return handle_peer_message(&msg);
}

void* p2p_server_function(void* arg) {
    (void) arg;

    struct ServerConfig config = {
        .port = utente_get_port(),

        .client_message_callback = handle_peer,
        .stdin_message_callback = NULL,
    };
    struct Server* peer = server_create(config);
    if (!peer) {
        utente_update_state(STATE_SHUTTING_DOWN);
        pthread_exit(0);
    }

    if (server_init(peer) < 0) {
        server_shutdown(peer);
        utente_update_state(STATE_SHUTTING_DOWN);
        pthread_exit(0);
    }

    utente_update_state(STATE_CONNECTING);
    
    while (utente_get_state() != STATE_DISCONNECTING && server_run(peer) == 0)
        ;

    server_shutdown(peer);

    utente_update_state(STATE_SHUTTING_DOWN);
    pthread_exit(0);
}