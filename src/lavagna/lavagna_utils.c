#include "../../include/common.h"
#include "../../include/lavagna_utils.h"
#include "../../include/database.h"
#include "../../include/protocol.h"
#include "../../include/server.h"

void send_user_list(int fd) {
    in_port_t* user_ports;

    int count = database_get_user_list(&user_ports);
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


void assign_card() {
    int next_card_id = database_get_next_todo_card();
    int next_user_port = database_get_next_user_port();

    if (next_card_id == -1 || next_user_port == -1) {
        return;
    }

    fprintf(stdout, "Assigning card %d to user on port %d\n", next_card_id, next_user_port);

    char buffer[MAX_PAYLOAD_SIZE];
    int32_t id_network = htonl(next_card_id);

    memcpy(buffer, &id_network, sizeof(id_network));
    char* text_ptr = buffer + sizeof(id_network);

    database_card_get_text(next_card_id, text_ptr, MAX_PAYLOAD_SIZE - sizeof(id_network));

    struct Message msg = {
        .type = MSG_HANDLE_CARD,
        .payload_length = sizeof(id_network) + strlen(text_ptr) + 1,
        .payload = buffer,
    };

    send_message(database_get_fd_from_port(next_user_port), &msg);

    database_user_set_status(next_user_port, USER_STATE_WAITING_ACK);
    database_user_assign_card(next_user_port, next_card_id);
}