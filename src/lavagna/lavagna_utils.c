#include "../../include/common.h"
#include "../../include/lavagna_utils.h"
#include "../../include/database.h"
#include "../../include/protocol.h"
#include "../../include/server.h"

int send_user_list(int socket) {
    in_port_t* user_ports;

    int count = database_get_users_list(&user_ports);
    if (count <= 0) {
        return -1;
    }

    for (int i = 0; i < count; i++) {
        user_ports[i] = htons(user_ports[i]);
    }
    
    struct Message msg = {
        .type = MSG_SEND_USER_LIST,
        .payload_length = count * sizeof(in_port_t),
        .payload = user_ports,
    };

    if (send_message(socket, &msg) < 0) {
        free(user_ports);
        return -1;
    }

    free(user_ports);
    return 0;
}


int send_user_ping(int socket) {
    in_port_t user_port = database_get_port_from_socket(socket);
    if (user_port == 0) {
        return -1;
    }

    if (database_user_get_status(user_port) != USER_STATUS_ACTIVE) {
        return -1;
    }

    struct Message msg = {
        .type = MSG_PING_USER,
        .payload_length = 0,
        .payload = NULL,
    };

    if (send_message(socket, &msg) < 0) {
        return -1;
    }

    database_user_set_ping(user_port);

    fprintf(stdout, "Sent PING to user on port %d\n", user_port);
    return 0;
}

int send_user_quit(int socket) {
    struct Message msg = {
        .type = MSG_QUIT,
        .payload_length = 0,
        .payload = NULL,
    };

    if (send_message(socket, &msg) < 0) {
        return -1;
    }

    fprintf(stdout, "Timeout. Sent QUIT to user on socket %d\n", socket);
    return 0;
}

int assign_card() {
    // Prima controllo se esiste una catrd da assegnare
    int next_card_id = database_get_next_todo_card();
    if (next_card_id == -1) {
        return -1;
    }

    // Poi controllo se c'è un utente idle a cui assegnarla
    int next_user_port = database_get_next_user_port();
    if (next_user_port == -1) {
        return -1;
    }

    fprintf(stdout, "Assigning card %d to user on port %d\n", next_card_id, next_user_port);

    // Preparo il messaggio e lo invio
    // È strutturato avente i primi 4 byte come l'id della card in network byte order
    // seguiti dal testo della card

    char buffer[MAX_PAYLOAD_SIZE];
    int32_t id_network = htonl(next_card_id);
    memcpy(buffer, &id_network, sizeof(id_network));
    char* text_ptr = buffer + sizeof(id_network);
    if (database_card_get_text(next_card_id, text_ptr, MAX_PAYLOAD_SIZE - sizeof(id_network)) < 0) {
        return -1;
    }

    struct Message msg = {
        .type = MSG_HANDLE_CARD,
        .payload_length = sizeof(id_network) + strlen(text_ptr) + 1,
        .payload = buffer,
    };

    if (send_message(database_get_socket_from_port(next_user_port), &msg) < 0) {
        return -1;
    }

    // Sappiamo già che l'utente esiste
    database_user_assign_card(next_user_port, next_card_id);

    return 0;
}

void disconnect_user(in_port_t user_port) {
    database_card_todo(user_port);
    database_remove_user(user_port);

    assign_card();
}

static void check_working_timeout() {
    int* timed_out_cards = NULL;

    int count = database_card_get_timed_out(&timed_out_cards, WORKING_TIMEOUT_SECONDS);
    if (count == 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        int card_id = timed_out_cards[i];

        in_port_t user_port = database_card_get_user(card_id);
        int user_socket = database_get_socket_from_port(user_port);
        if (user_socket != -1) {
            send_user_ping(user_socket);
        }
    }

    free(timed_out_cards);
}

static void check_ping_timeout() {
    in_port_t* timed_out_users = NULL;

    int count = database_user_get_timed_out(&timed_out_users, PING_TIMEOUT_SECONDS);
    if (count == 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        in_port_t user_port = timed_out_users[i];
        fprintf(stdout, "User on port %d has timed out after ping. Removing user and reassigning card.\n", user_port);

        int user_socket = database_get_socket_from_port(user_port);

        send_user_quit(user_socket);
        disconnect_user(user_port);
    }

    free(timed_out_users);
}

void check_timeout() {
    check_working_timeout();
    check_ping_timeout();
}