#include "pch.h"
#include "server.h"
#include "p2p_thread.h"
#include "client.h"

void* p2p_server_function(void* arg) {
    Client* client = (Client*) arg;

    ServerConfig config = {
        .port = client->port,
        .client_handler = NULL,
        .stdin_handler = NULL,
    };
    Server* peer = server_create(config);
    if (!peer) {
        client_update_state(client, STATE_SHUTTING_DOWN);
        pthread_exit(0);
    }

    if (server_start(peer) < 0) {
        client_update_state(client, STATE_CONNECTING);
        server_run(peer);
    }

    server_run(peer);

    server_shutdown(peer);
    client_update_state(client, STATE_SHUTTING_DOWN);
    pthread_exit(0);
}