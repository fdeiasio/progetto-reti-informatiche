#include "server.h"
#include "p2p_thread.h"

void* p2p_server_function(void* arg) {
    in_port_t port = *(in_port_t*) arg;

    ServerConfig config = {
        .port = port,
        .new_client_handler = NULL,
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

    pthread_exit(0);
}