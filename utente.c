#include "lib/pch.h"
#include "lib/p2p_thread.h"
#include "lib/protocol.h"
#include "lib/client.h"

#define MIN_PORT 5679
#define SERVER_PORT 5678
#define SERVER_IP "127.0.0.1"

static int running = 1;
void signal_handler(int signum) {
    (void)signum;
    
    running = 0;
}

int send_server_hello(struct Client* client) {
    in_port_t port_network = htons(client->port);
    struct Message msg = {
        .type = MSG_HELLO,
        .payload_length = sizeof(port_network),
        .payload = &port_network,
    };

    if (send_message(client->server_socket, &msg) < 0) {
        fprintf(stderr, "Error: Could not send HELLO message to server.\n");
        return -1;
    } 

    fprintf(stdout, "Sent HELLO message to server\n");
    return 0;
}

int handle_stdin_message(struct Client* client, struct Message* msg) {
    switch (msg->type) {
        case MSG_HELLO:
            send_server_hello(client);
            break;

        case MSG_QUIT:
            return -1;

        case MSG_CREATE_CARD:
            if (msg->payload_length == 0) {
                fprintf(stderr, "Error: CREATE_CARD requires an argument.\n");
                return 0;
            }
            if (send_message(client->server_socket, msg) < 0) {
                fprintf(stderr, "Error: Could not send CREATE_CARD message to server.\n");
                return 0;
            }
            
            fprintf(stdout, "Sent CREATE_CARD message to server\n");
            break;

        default:
            fprintf(stderr, "Error: Unknown message type.\n");
            break;
    }
    return 0;
}

int handle_stdin(struct Client* client) {
    char buffer[MAX_PAYLOAD_SIZE];
    struct Message msg = {
        .payload = buffer,
        .payload_length = MAX_PAYLOAD_SIZE,
    };
    
    if (get_command_line_input(&msg) < 0) {
        return 0;
    }

    return handle_stdin_message(client, &msg);
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

    struct ClientConfig config = {
        .client_port = port,
        .server_port = SERVER_PORT,
        .server_handler = NULL,
        .stdin_handler = handle_stdin,
    };
    inet_pton(AF_INET, SERVER_IP, &config.server_ip);

    struct Client* client = client_create(config);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    client_start_p2p(client, p2p_server_function);

    while (running && client->state != STATE_SHUTTING_DOWN) {
        switch (client->state) {
            case STATE_STARTING_P2P:
                // Waiting for P2P server to start
                break;

            case STATE_CONNECTING:
                client_connect_to_server(client);

                if (send_server_hello(client) < 0) {
                    client_update_state(client, STATE_SHUTTING_DOWN);
                    break;
                }

                client_update_state(client, STATE_IDLE);
                break;

            case STATE_IDLE:

                if (client_listen(client) < 0) {
                    client_update_state(client, STATE_DISCONNECTING);
                }
                break;

            case STATE_DISCONNECTING:
                // Wait for shutdown signal from p2p thread
                break;
        }
    }

    client_disconnect_from_server(client);
    client_stop_p2p(client);
    
    free(client);
}