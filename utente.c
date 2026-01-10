#include "lib/pch.h"
#include "lib/p2p_thread.h"
#include "lib/protocol.h"
#include "lib/client.h"

#define MIN_PORT 5679
#define SERVER_PORT 5678

static int running = 1;
void signal_handler(int signum) {
    (void)signum;
    
    running = 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    in_port_t port = atoi(argv[1]);
    if (port < MIN_PORT) {
        fprintf(stderr, "Error: Invalid port number.\n");
        exit(1);
    }  

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    Client* client = client_create(port);

    client_start_p2p(client, p2p_server_function);

    while (running && client->state != STATE_SHUTTING_DOWN) {
        switch (client->state) {
            case STATE_STARTING_P2P:
                // Waiting for P2P server to start
                break;

            case STATE_CONNECTING:
                client_connect_to_server(client, "127.0.0.1", SERVER_PORT);
                client_send_server_hello(client);

                client_update_state(client, STATE_IDLE);
                break;

            case STATE_IDLE:
                break;
        }
    }

    client_update_state(client, STATE_DISCONNECTING);

    pthread_join(client->p2p_server_thread, NULL);
    
    free(client);
}