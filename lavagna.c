#include "include/common.h"
#include "include/server.h"
#include "include/protocol.h"
#include "include/database.h"

#define MAX_USERS 100

static volatile sig_atomic_t active = 1;

void signal_handler(int signum) {
    (void)signum;
    
    active = 0;
}

void send_user_list(int fd) {

    int num_users = database_get_num_users();

    in_port_t* user_ports = (in_port_t*)malloc(num_users * sizeof(in_port_t));
    if (!user_ports) {
        fprintf(stderr, "Error: Could not allocate memory for user ports.\n");
        return;
    }

    int count = database_get_user_list(user_ports, num_users);
    for (int i = 0; i < count; i++) {
        user_ports[i] = htons(user_ports[i]);
    }
    
    struct Message msg = {
        .type = MSG_SEND_USER_LIST,
        .payload_length = count * sizeof(in_port_t),
        .payload = user_ports,
    };

    send_message(fd, &msg);
    free(user_ports);

    fprintf(stdout, "Sent user list to client\n");
}

int assign_card() {
    int next_card_id = database_get_next_todo_card();
    int next_user_port = database_get_next_user_port();

    if (next_card_id == -1 || next_user_port == -1) {
        return -1;
    }

    fprintf(stdout, "Assigning card %d to user on port %d\n", next_card_id, next_user_port);

    char buffer[MAX_PAYLOAD_SIZE];
    int32_t id_network = htonl(next_card_id);

    memcpy(buffer, &id_network, sizeof(id_network));
    char* text_ptr = buffer + sizeof(id_network);

    if (database_card_get_text(next_card_id, text_ptr, MAX_PAYLOAD_SIZE - sizeof(id_network)) < 0) {
        fprintf(stderr, "Error: Could not get card text.\n");
        return -1;
    }

    struct Message msg = {
        .type = MSG_HANDLE_CARD,
        .payload_length = sizeof(id_network) + strlen(text_ptr) + 1,
        .payload = buffer,
    };

    if (send_message(database_get_fd_from_port(next_user_port), &msg) < 0) {
        fprintf(stderr, "Error: Could not send HANDLE_CARD message to user on port %d.\n", next_user_port);
        return -1;
    }

    database_user_set_status(next_user_port, USER_STATE_WAITING_ACK);
    database_user_assign_card(next_user_port, next_card_id);

    return 0;
}

int handle_new_client(struct Server* server, void* args) {
    int fd = *(int*)args;
    
    fprintf(stdout, "New client connected\n");

    return 0;
}

int handle_client_message(struct Server* server, int fd, struct Message* msg) {
    switch (msg->type) {
        case MSG_HELLO: {
            in_port_t user_port = ntohs(*(in_port_t*) msg->payload);
            fprintf(stdout, "Received HELLO message from user on port %d\n", user_port);

            struct User new_user = {
                .fd = fd,
                .port = user_port,
            };

            if (database_add_user(&new_user) < 0) {
                fprintf(stderr, "Error: Could not add user to database.\n");
            }
            database_print_users();

            assign_card();
            break;
        }

        case MSG_CREATE_CARD: {
            int user_port = database_get_port_from_fd(fd);
            fprintf(stdout, "Received CREATE_CARD message from user on port %d\n", user_port);

            char* card_text = (char*) msg->payload;
            if (database_create_card(card_text) < 0) {
                fprintf(stderr, "Error: Could not create card.\n");
            }
            database_print_cards();

            assign_card();
            break;
        }

        case MSG_REQUEST_USER_LIST:
            fprintf(stdout, "Received REQUEST_USER_LIST message from user on port %d\n", 
                database_get_port_from_fd(fd)
            );

            send_user_list(fd);
            break;

        case MSG_ACK_CARD: {
            int user_port = database_get_port_from_fd(fd);
            fprintf(stdout, "Received ACK_CARD message from user on port %d\n", user_port);
            database_user_set_status(user_port, USER_STATE_WORKING);

            database_card_doing(user_port);
            database_print_cards();
        }   
            break;

        case MSG_QUIT: {
            int user_port = database_get_port_from_fd(fd);
            fprintf(stdout, "Received QUIT message from user on port %d\n", user_port);

            database_card_todo(user_port);
            database_remove_user(user_port);
            database_print_users();
            database_print_cards();

            assign_card();
            break;
        }
        default:
            fprintf(stderr, "Error: Unknown message type from user %d\n", fd);
            break;
    }

    return 0;
}

int handle_client(struct Server* server, void* args) {
    int fd = *(int*) args;

    char buffer[MAX_PAYLOAD_SIZE];
    memset(buffer, 0, MAX_PAYLOAD_SIZE);

    struct Message msg = {
        .payload = buffer,
        .payload_length = MAX_PAYLOAD_SIZE,
    };

    ssize_t bytes_received = receive_message(fd, &msg);
    if (bytes_received <= 0) {
        int port = database_get_port_from_fd(fd);

        if (port != -1) {
            fprintf(stdout, "Client %d disconnected\n", port);
            database_remove_user(port);
            database_card_todo(port);
            assign_card();
        } 
        else {
            fprintf(stdout, "Unknown client disconnected\n");
        }
        
        database_print_users();
        return -1;
    }

    return handle_client_message(server, fd, &msg);
}

int handle_stdin_message(struct Server* server, struct Message* msg) {
    switch (msg->type) {
        case MSG_SHOW_UTENTI:
            database_print_users();
            break;

        case MSG_SHOW_LAVAGNA:
            database_print_cards();
            break;

        default:
            fprintf(stderr, "Error: Unknown command.\n");
            break;
    }
    return 0;
}

int handle_stdin(struct Server* server, void* args) {
    (void) args;

    char buffer[MAX_PAYLOAD_SIZE];
    struct Message msg = {
        .payload = buffer,
        .payload_length = MAX_PAYLOAD_SIZE,
    };
    
    if (get_command_line_input(&msg) < 0) {
        return 0;
    }

    return handle_stdin_message(server, &msg);
}

int main() {
    struct DatabaseConfig db_config = {
        .max_users = MAX_USERS,
    }; 
    if (database_init(db_config) < 0) {
        exit(1);
    }

    struct ServerConfig config = {
        .port = SERVER_PORT,
        .new_client_handler = handle_new_client,
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
    while(active && server_run(lavagna) == 0)
        ;

    server_shutdown(lavagna);
    database_cleanup();
    exit(0);
}