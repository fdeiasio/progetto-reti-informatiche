#include "pch.h"
#include "server.h"
#include "p2p_thread.h"
#include "client.h"

int handle_new_peer(Server* server, void* args) {
    int fd = *(int*)args;
    
    fprintf(stdout, "New peer connected\n");

    return 0;
}

void* p2p_server_function(void* arg) {
    Client* client = (Client*) arg;

    ServerConfig config = {
        .port = client->port,
        .new_client_handler = handle_new_peer,
        .client_handler = NULL,
        .stdin_handler = NULL,
    };
    Server* peer = server_create(config);
    if (!peer) {
        client_update_state(client, STATE_SHUTTING_DOWN);
        pthread_exit(0);
    }

    if (server_init(peer) < 0) {
        server_shutdown(peer);
        client_update_state(client, STATE_SHUTTING_DOWN);
        pthread_exit(0);
    }

    client_update_state(client, STATE_CONNECTING);
    
    while (client->state != STATE_DISCONNECTING && server_run(peer) == 0)
        ;

    server_shutdown(peer);
    client_update_state(client, STATE_SHUTTING_DOWN);
    pthread_exit(0);
}