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
            if (send_server_hello(server_fd) < 0) {
                fprintf(stderr, "Error: Could not send HELLO message to server.\n");
                return -1;
            }
            break;

        case MSG_QUIT:
            send_message(server_fd, msg);
            return -1;

        case MSG_CREATE_CARD:
            if (msg->payload_length == 0) {
                fprintf(stderr, "Error: CREATE_CARD requires an argument.\n");
                return 0;
            }

            if (send_message(server_fd, msg) < 0) {
                fprintf(stderr, "Error: Could not send CREATE_CARD message to server.\n");
                return -1;
            }
            break;

        case MSG_REQUEST_USER_LIST:
            if (send_message(server_fd, msg) < 0) {
                fprintf(stderr, "Error: Could not send REQUEST_USER_LIST message to server.\n");
                return -1;
            }
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

            if (utente_allocate_user_list(&user_ports, num_users) < 0) {
                fprintf(stderr, "Error: Could not allocate memory for user list.\n");
                return -1;
            }

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
            if (utente_start_worker(worker_thread_function) < 0) {
                fprintf(stderr, "Error: Could not start worker thread.\n");
                return -1;
            }

            if (send_server_card_ack(server_fd) < 0) {
                fprintf(stderr, "Error: Could not send CARD_ACK to server.\n");
                return -1;
            }
            break;

        case MSG_PING_USER:
            fprintf(stdout, "Received PING_USER from server.\n");
            
            if (send_server_pong(server_fd) < 0) {
                fprintf(stderr, "Error: Could not send PONG to server.\n");
                return -1;
            }
            
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

    fprintf(stdout, "Starting P2P server thread...\n");

    if (utente_start_p2p(p2p_server_function) < 0) {
        fprintf(stderr, "Error: Could not start P2P server thread.\n");
        client_destroy(client);
        utente_cleanup();
        exit(1);
    }

    while (running && utente_get_state() != STATE_SHUTTING_DOWN) {
        enum UtenteState current_state = utente_get_state();

        switch (current_state) {
            case STATE_STARTING_P2P:
                // Aspetto che il server P2P sia avviato
                break;

            case STATE_CONNECTING:
                // Mi connetto al server centrale e invio il messaggio HELLO
                if (client_connect_to_server(client) < 0) {
                    fprintf(stderr, "Error: Could not connect to server.\n");
                    utente_update_state(STATE_DISCONNECTING);
                    break;
                }

                fprintf(stdout, "Sending HELLO message to server.\n");

                if (send_server_hello(client->server_socket) < 0) {
                    fprintf(stderr, "Error: Could not send HELLO message to server.\n");
                    utente_update_state(STATE_DISCONNECTING);
                    break;
                }

                utente_update_state(STATE_IDLE);
                break;

            case STATE_IDLE:
                if (client_listen(client) < 0) {
                    utente_update_state(STATE_DISCONNECTING);
                }
                break;

            case STATE_WORKING:
                // Il thread worker sta lavorando
                if (client_listen(client) < 0) {
                    utente_update_state(STATE_DISCONNECTING);
                }
                break;

            case STATE_REQUESTING_UL:
                // Il thread worker ha finito e ora chiedo la user list al server
                utente_wait_for_worker_shutdown();
                
                struct Message msg = {
                    .type = MSG_REQUEST_USER_LIST,
                    .payload_length = 0,
                    .payload = NULL,
                };

                if (send_message(client->server_socket, &msg) < 0) {
                    fprintf(stderr, "Error: Could not send REQUEST_USER_LIST message to server.\n");
                    utente_update_state(STATE_DISCONNECTING);
                    break;
                }

                utente_update_state(STATE_WAITING_UL);
                break;

            case STATE_WAITING_UL:
                // Aspetto la user list dal server

                if (client_listen(client) < 0) {
                    utente_update_state(STATE_DISCONNECTING);
                }
                break;

            case STATE_WAITING_ACK:
                // Aspetto che il server p2p riceva un ack dai peer

                if (client_listen(client) < 0) {
                    utente_update_state(STATE_DISCONNECTING);
                }
                break;

            case STATE_DONE_WORK:
                // Mando il card done e torno in idle
                fprintf(stdout, "Review completed.\n");
                
                utente_set_card_id(-1);

                struct Message done_msg = {
                    .type = MSG_CARD_DONE,
                    .payload_length = 0,
                    .payload = NULL,
                };

                if (send_message(client->server_socket, &done_msg) < 0) {
                    fprintf(stderr, "Error: Could not send CARD_DONE message to server.\n");
                    utente_update_state(STATE_DISCONNECTING);
                    break;
                }

                utente_update_state(STATE_IDLE);
                break;
            case STATE_DISCONNECTING:
                // Aspetto il segnale di spegnimento dal thread p2p
                break;

            default:
                break;
        }
    }

    fprintf(stdout, "Shutting down...\n");

    utente_wait_for_p2p_shutdown();

    client_destroy(client);
    utente_cleanup();
    exit(0);
}