#include "../../include/common.h"
#include "../../include/server.h"
#include "../../include/protocol.h"
#include "../../include/utente_state.h"
#include "../../include/p2p_utils.h"

#define LOCALHOST "127.0.0.1"

static int remaining_acks = 0;

int handle_new_peer(struct Server* server, void* args) {

    return 0;
}

int handle_peer_message(struct Server* server, int fd, struct Message* msg) {
    switch (msg->type) {
        case MSG_SEND_USER_LIST: {
            remaining_acks = p2p_broadcast_review_request(server);
            
            if (remaining_acks == 0) {
                utente_update_state(STATE_DONE_WORK);
            }
            break;
        }

        case MSG_REVIEW_CARD: {
            fprintf(stdout, "P2P: Received review request from peer.\n");

            struct Message response = {
                .type = MSG_DONE_REVIEW,
                .payload_length = 0,
                .payload = NULL,
            };

            send_message(fd, &response);

            break;
        }

        case MSG_DONE_REVIEW: {
            fprintf(stdout, "P2P: Received done review from peer.\n");

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

int handle_peer(struct Server* server, void* args) {
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

    return handle_peer_message(server, fd, &msg);
}

void* p2p_server_function(void* arg) {
    (void) arg;

    struct ServerConfig config = {
        .port = utente_get_port(),
        .new_client_handler = handle_new_peer,
        .client_handler = handle_peer,
        .stdin_handler = NULL,
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