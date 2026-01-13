#include "include/common.h"
#include "include/thread.h"
#include "include/protocol.h"
#include "include/client.h"
#include "include/utente_state.h"
#include "include/utente_utils.h"

#define MIN_PORT 5679

static volatile sig_atomic_t running = 1;

void signal_handler(int signum) {
    (void)signum;
    
    running = 0;
}

int handle_stdin_message(int server_fd, struct Message* msg) {
    switch (msg->type) {
        case MSG_HELLO:
            send_server_hello(server_fd);
            break;

        case MSG_QUIT:
            send_message(server_fd, msg);
            return -1;

        case MSG_CREATE_CARD:
            if (msg->payload_length == 0) {
                fprintf(stderr, "Error: CREATE_CARD requires an argument.\n");
                return 0;
            }

            send_message(server_fd, msg);
            break;

        case MSG_REQUEST_USER_LIST:
            send_message(server_fd, msg);
            break;

        default:
            fprintf(stderr, "Error: Unknown message type.\n");
            break;
    }
    return 0;
}

int handle_stdin(void* args) {
    int server_fd = *(int*)args;

    static char buffer[MAX_PAYLOAD_SIZE];
    struct Message msg = {
        .payload = buffer,
        .payload_length = MAX_PAYLOAD_SIZE,
    };

    if (get_command_line_input(&msg) < 0) {
        return 0;
    }

    return handle_stdin_message(server_fd, &msg);
}

int handle_server_message(int server_fd, struct Message* msg) {
    switch (msg->type) {
        case MSG_SEND_USER_LIST: {
            fprintf(stdout, "Received peer list from server:\n");

            // Parse the message
            int num_users = msg->payload_length / sizeof(in_port_t);
            in_port_t* user_ports = (in_port_t*)msg->payload;

            for (int i = 0; i < num_users; i++) {
                user_ports[i] = ntohs(user_ports[i]);
            }

            utente_allocate_user_list(&user_ports, num_users);

            if (utente_get_state() == STATE_WAITING_UL) {
                utente_update_state(STATE_WAITING_ACK);

                send_p2p_user_list();
            }

            break;
        }

        case MSG_HANDLE_CARD:
            fprintf(stdout, "Server assigned you a card to handle.\n");

            if (utente_get_state() != STATE_IDLE) {
                break;
            }

            int card_id = ntohl(*(uint32_t*)msg->payload);
            utente_set_card_id(card_id);

            utente_update_state(STATE_WORKING);
            utente_start_worker(worker_thread_function);

            send_server_card_ack(server_fd);
            break;

        case MSG_PING_USER:
            fprintf(stdout, "Received PING_USER from server.\n");
            
            send_server_pong(server_fd);
            
            break;

        case MSG_QUIT:
            fprintf(stdout, "Timed out.\n");
            return -1;

        default:
            fprintf(stderr, "Error: Unknown message type from server.\n");
            break;
    }
    return 0;
}

int handle_server(void* args) {
    int server_fd = *(int*)args;

    char buffer[MAX_PAYLOAD_SIZE];
    memset(buffer, 0, MAX_PAYLOAD_SIZE);

    struct Message msg = {
        .payload = buffer,
        .payload_length = MAX_PAYLOAD_SIZE,
    };

    ssize_t bytes_received = receive_message(server_fd, &msg);
    if (bytes_received <= 0) {
        fprintf(stdout, "Disconnected from server\n");
        return -1;
    }

    return handle_server_message(server_fd, &msg);
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

    utente_init(port);

    struct ClientConfig config = {
        .server_port = SERVER_PORT,

        .server_message_callback = handle_server,
        .stdin_message_callback = handle_stdin,
    };
    inet_pton(AF_INET, LOCALHOST, &config.server_ip);

    struct Client* client = client_create(config);
    if (!client) {
        fprintf(stderr, "Error: Could not create client.\n");
        exit(1);
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    utente_start_p2p(p2p_server_function);

    while (running && utente_get_state() != STATE_SHUTTING_DOWN) {
        enum UtenteState current_state = utente_get_state();

        switch (current_state) {
            case STATE_STARTING_P2P:
                // Waiting for P2P server to start
                break;

            case STATE_CONNECTING:
                client_connect_to_server(client);

                send_server_hello(client->server_socket);

                utente_update_state(STATE_IDLE);
                break;

            case STATE_IDLE:

                if (client_listen(client) < 0) {
                    utente_update_state(STATE_DISCONNECTING);
                }
                break;

            case STATE_WORKING:

                if (client_listen(client) < 0) {
                    utente_update_state(STATE_DISCONNECTING);
                }
                break;

            case STATE_REQUESTING_UL:
                utente_wait_for_worker_shutdown();
                
                struct Message msg = {
                    .type = MSG_REQUEST_USER_LIST,
                    .payload_length = 0,
                    .payload = NULL,
                };

                send_message(client->server_socket, &msg);

                utente_update_state(STATE_WAITING_UL);
                break;

            case STATE_WAITING_UL:

                if (client_listen(client) < 0) {
                    utente_update_state(STATE_DISCONNECTING);
                }
                break;

            case STATE_WAITING_ACK:

                if (client_listen(client) < 0) {
                    utente_update_state(STATE_DISCONNECTING);
                }
                break;

            case STATE_DONE_WORK:
                fprintf(stdout, "Review completed.\n");
                
                utente_set_card_id(-1);

                struct Message done_msg = {
                    .type = MSG_CARD_DONE,
                    .payload_length = 0,
                    .payload = NULL,
                };
                send_message(client->server_socket, &done_msg);

                utente_update_state(STATE_IDLE);
                break;
            case STATE_DISCONNECTING:
                // Wait for shutdown signal from p2p thread
                break;

            default:
                break;
        }
    }

    utente_wait_for_p2p_shutdown();

    client_destroy(client);
    utente_cleanup();
    exit(0);
}