#include "include/common.h"
#include "include/server.h"
#include "include/protocol.h"
#include "include/database.h"
#include "include/lavagna_utils.h"

static volatile sig_atomic_t active = 1;

void signal_handler(int signum) {
    (void)signum;
    
    active = 0;
}

int handle_client_message(int fd, struct Message* msg) {
    switch (msg->type) {
        case MSG_HELLO: {
            in_port_t user_port = ntohs(*(in_port_t*) msg->payload);
            fprintf(stdout, "Received HELLO message from user on port %d\n", user_port);

            database_add_user(fd, user_port);
            database_print_users();

            int pending_fd = database_get_pending_user_socket();
            if (pending_fd != -1) {
                send_user_list(pending_fd);
                database_user_clear_pending();
            }

            assign_card();
            break;
        }

        case MSG_CREATE_CARD: {
            int user_port = database_get_port_from_socket(fd);
            fprintf(stdout, "Received CREATE_CARD message from user on port %d\n", user_port);

            char* card_text = (char*) msg->payload;
            if (database_create_card(card_text) < 0) {
                fprintf(stderr, "Error: Could not create card.\n");
            }

            assign_card();
            break;
        }

        case MSG_REQUEST_USER_LIST:
            fprintf(stdout, "Received REQUEST_USER_LIST message from user on port %d\n", 
                database_get_port_from_socket(fd)
            );

            if (database_get_num_users() == 1) {
                database_user_set_pending(fd);
                break;
            }

            send_user_list(fd);
            break;

        case MSG_ACK_CARD: {
            int user_port = database_get_port_from_socket(fd);
            fprintf(stdout, "Received ACK_CARD message from user on port %d\n", user_port);

            database_card_doing(user_port);
            database_print_cards();
        }   
            break;

        case MSG_CARD_DONE: {
            int user_port = database_get_port_from_socket(fd);
            fprintf(stdout, "Received CARD_DONE message from user on port %d\n", user_port);

            database_card_done(user_port);
            database_user_set_status(user_port, USER_STATUS_IDLE);

            assign_card();
            database_print_cards();
            break;
        }

        case MSG_QUIT: {
            int user_port = database_get_port_from_socket(fd);
            fprintf(stdout, "Received QUIT message from user on port %d\n", user_port);

            disconnect_user(user_port);
            break;
        }
        case MSG_PONG_LAVAGNA: {
            int user_port = database_get_port_from_socket(fd);
            database_card_reset_timestamp(user_port);
            database_user_clear_ping(user_port);

            break;
        }

        default:
            fprintf(stderr, "Error: Unknown message type from user %d\n", fd);
            break;
    }

    return 0;
}

int handle_client(void* args) {
    int fd = *(int*)args;

    char buffer[MAX_PAYLOAD_SIZE];
    memset(buffer, 0, MAX_PAYLOAD_SIZE);

    struct Message msg = {
        .payload = buffer,
        .payload_length = MAX_PAYLOAD_SIZE,
    };

    ssize_t bytes_received = receive_message(fd, &msg);
    if (bytes_received <= 0) {
        int port = database_get_port_from_socket(fd);

        if (port != -1) {
            fprintf(stdout, "Client %d disconnected\n", port);
            database_card_todo(port);
            database_remove_user(port);

            assign_card();
        } 
        
        database_print_users();
        return -1;
    }

    if (database_get_port_from_socket(fd) == 0 && msg.type != MSG_HELLO) {
        fprintf(stderr, "Error: Received message from unknown client\n");
        return -1;
    }

    return handle_client_message(fd, &msg);
}

int handle_stdin_message(struct Message* msg) {
    switch (msg->type) {
        case MSG_SHOW_UTENTI:
            database_print_users();
            break;

        case MSG_SHOW_LAVAGNA:
            database_print_cards();
            break;

        case MSG_PING_USER:
            if (msg->payload_length == 0) {
                fprintf(stderr, "Error: CREATE_CARD requires an argument.\n");
                return 0;
            }

            char buffer[4];
            memcpy(buffer, msg->payload, 4);

            in_port_t user_port = atoi(buffer);

            int fd = database_get_socket_from_port(user_port);
            if (fd == -1) {
                fprintf(stderr, "Error: No user found on port %d.\n", user_port);
                return 0;
            }

            if (send_user_ping(fd) < 0) {
                fprintf(stderr, "Can't send ping, user is not working\n");
                return 0;
            }
            break;

        default:
            fprintf(stderr, "Error: Unknown command.\n");
            break;
    }
    return 0;
}

int handle_stdin(void* args) {
    (void) args;

    char buffer[MAX_PAYLOAD_SIZE];
    struct Message msg = {
        .payload = buffer,
        .payload_length = MAX_PAYLOAD_SIZE,
    };
    
    if (get_command_line_input(&msg) < 0) {
        return 0;
    }

    return handle_stdin_message(&msg);
}

int main() {
    database_init();

    struct ServerConfig config = {
        .port = SERVER_PORT,
        
        .client_handler = handle_client,
        .stdin_handler = handle_stdin,
    };
    struct Server* lavagna = server_create(config);
    if (!lavagna) {
        database_cleanup();
        exit(1);
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (server_init(lavagna) < 0) {
        server_shutdown(lavagna);
        database_cleanup();
        exit(1);
    }

    fprintf(stdout, "Lavagna server started on port %d\n", SERVER_PORT);
    while(active && server_run(lavagna) == 0) {
        if (time(NULL) % TIMEOUT_CHECK_PERIOD == 0) {
            check_timeout();
        }
    }

    server_shutdown(lavagna);
    database_cleanup();
    exit(0);
}