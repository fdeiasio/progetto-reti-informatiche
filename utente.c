#include "include/common.h"
#include "include/thread.h"
#include "include/protocol.h"
#include "include/client.h"
#include "include/utente_state.h"
#include "include/utente_utils.h"

static volatile sig_atomic_t running = 1;

void signal_handler(int signum) {
    (void)signum;
    
    running = 0;
}

int handle_stdin_message(struct Client* client, struct Message* msg) {
    switch (msg->type) {
        case MSG_HELLO:
            send_server_hello(client);
            break;

        case MSG_QUIT:
            send_message(client->server_socket, msg);
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

        case MSG_REQUEST_USER_LIST:
            if (send_message(client->server_socket, msg) < 0) {
                fprintf(stderr, "Error: Could not send REQUEST_USER_LIST message to server.\n");
                return 0;
            }

            fprintf(stdout, "Sent REQUEST_USER_LIST message to server\n");
            break;

        default:
            fprintf(stderr, "Error: Unknown message type.\n");
            break;
    }
    return 0;
}

int handle_stdin(struct Client* client) {
    static char buffer[MAX_PAYLOAD_SIZE];
    struct Message msg = {
        .payload = buffer,
        .payload_length = MAX_PAYLOAD_SIZE,
    };

    if (get_command_line_input(&msg) < 0) {
        return 0;
    }

    return handle_stdin_message(client, &msg);
}

int handle_server_message(struct Client* client, struct Message* msg) {
    switch (msg->type) {
        case MSG_SEND_USER_LIST: {
            fprintf(stdout, "Received peer list from server:\n");

            int num_users = msg->payload_length / sizeof(in_port_t);
            in_port_t* user_ports = (in_port_t*)msg->payload;

            if (num_users == 0) {
                fprintf(stdout, "(no users)\n");
            } else {
                for (int i = 0; i < num_users; i++) {
                    fprintf(stdout, "- User on port %d\n", ntohs(user_ports[i]));
                }
            }

            if (utente.user_list) {
                free(utente.user_list);
            }
            utente.user_list = malloc(num_users * sizeof(in_port_t));
            if (!utente.user_list) {
                fprintf(stderr, "Error: Could not allocate memory for user list.\n");
                exit(1);
            }
            memcpy(utente.user_list, user_ports, num_users * sizeof(in_port_t));
            utente.num_users = num_users;

            if (utente.state == STATE_WAITING_UL) {
                utente_update_state(&utente, STATE_WAITING_ACK);
            }

            break;
        }

        case MSG_HANDLE_CARD:
            fprintf(stdout, "Server assigned you a card to handle.\n");

            if (utente.state != STATE_IDLE) {
                break;
            }

            utente.card_id = ntohl(*(uint32_t*)msg->payload);
            utente_update_state(&utente, STATE_WORKING);
            utente_start_worker(&utente, worker_thread_function);

            send_server_card_ack(client);

            break;

        default:
            fprintf(stderr, "Error: Unknown message type from server.\n");
            break;
    }
    return 0;
}

int handle_server(struct Client* client) {
    char buffer[MAX_PAYLOAD_SIZE];
    memset(buffer, 0, MAX_PAYLOAD_SIZE);

    struct Message msg = {
        .payload = buffer,
        .payload_length = MAX_PAYLOAD_SIZE,
    };

    ssize_t bytes_received = receive_message(client->server_socket, &msg);
    if (bytes_received <= 0) {
        fprintf(stdout, "Disconnected from server\n");
        return -1;
    }

    return handle_server_message(client, &msg);
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

    utente_init(&utente, port);

    struct ClientConfig config = {
        .server_port = SERVER_PORT,

        .server_handler = handle_server,
        .stdin_handler = handle_stdin,
    };
    inet_pton(AF_INET, LOCALHOST, &config.server_ip);

    struct Client* client = client_create(config);
    if (!client) {
        fprintf(stderr, "Error: Could not create client.\n");
        exit(1);
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    utente_start_p2p(&utente, p2p_server_function);

    while (running && utente.state != STATE_SHUTTING_DOWN) {
        switch (utente.state) {
            case STATE_STARTING_P2P:
                // Waiting for P2P server to start
                break;

            case STATE_CONNECTING:
                client_connect_to_server(client);

                send_server_hello(client);

                utente_update_state(&utente, STATE_IDLE);
                break;

            case STATE_IDLE:

                if (client_listen(client) < 0) {
                    utente_update_state(&utente, STATE_DISCONNECTING);
                }
                break;

            case STATE_WORKING:
                

                if (client_listen(client) < 0) {
                    utente_update_state(&utente, STATE_DISCONNECTING);
                }
                break;

            case STATE_REQUESTING_UL:
                pthread_join(utente.worker_thread, NULL);

                struct Message msg = {
                    .type = MSG_REQUEST_USER_LIST,
                    .payload_length = 0,
                    .payload = NULL,
                };

                send_message(client->server_socket, &msg);

                utente_update_state(&utente, STATE_WAITING_UL);
                break;

            case STATE_WAITING_UL:

                if (client_listen(client) < 0) {
                    utente_update_state(&utente, STATE_DISCONNECTING);
                }
                break;

            case STATE_WAITING_ACK:

                if (client_listen(client) < 0) {
                    utente_update_state(&utente, STATE_DISCONNECTING);
                }
                break;
            case STATE_DISCONNECTING:
                // Wait for shutdown signal from p2p thread
                break;
        }
    }

    client_destroy(client);

    utente_cleanup(&utente);
    exit(0);
}