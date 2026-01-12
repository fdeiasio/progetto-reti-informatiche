#include "../../include/common.h"
#include "../../include/server.h"
#include "../../include/protocol.h"
#include "../../include/thread.h"
#include "../../include/utente_state.h"

#define LOCALHOST "127.0.0.1"

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