#include "pch.h"
#include "server.h"
#include "p2p_thread.h"
#include "user.h"

void* p2p_server_function(void* arg) {
    User* user = (User*) arg;

    ServerConfig config = {
        .port = user->port,
        .client_handler = NULL,
        .stdin_handler = NULL,
    };

    Server* peer = server_create(config);
    if (server_start(peer) < 0) {
        server_shutdown(peer);
        pthread_exit(0);
    }

    server_run(peer);

    server_shutdown(peer);
    user->state = STATE_DISCONNECTING;
    
    pthread_exit(0);
}