#include "../../include/common.h"
#include "../../include/utente_utils.h"
#include "../../include/client.h"
#include "../../include/utente_state.h"
#include "../../include/protocol.h"

int send_server_hello(int server_fd) {
    in_port_t port_network = htons(utente_get_port());

    struct Message msg = {
        .type = MSG_HELLO,
        .payload_length = sizeof(port_network),
        .payload = &port_network,
    };

    if (send_message(server_fd, &msg) < 0) {
        return -1;
    } 

    return 0;
}

int send_server_card_ack(int server_fd) {
    struct Message msg = {
        .type = MSG_ACK_CARD,
        .payload_length = 0,
        .payload = NULL,
    };

    if (send_message(server_fd, &msg) < 0) {
        return -1;
    } 

    return 0;
}

int send_server_pong(int server_fd) {
    struct Message msg = {
        .type = MSG_PONG_LAVAGNA,
        .payload_length = 0,
        .payload = NULL,
    };

    if (send_message(server_fd, &msg) < 0) {
        return -1;
    }

    return 0;
}

int send_p2p_user_list() {
    int p2p_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (p2p_socket < 0) {
        return -1;
    }
    struct sockaddr_in p2p_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(utente_get_port()),
        .sin_addr.s_addr = inet_addr(LOCALHOST),
    };

    if (connect(p2p_socket, (struct sockaddr *)&p2p_addr, sizeof(p2p_addr)) < 0) {
        close(p2p_socket);
        return -1;
    }

    // Il payload è nella struttura globale utente
    struct Message msg = {
        .type = MSG_SEND_USER_LIST,
        .payload_length = 0,
        .payload = NULL,
    };

    if (send_message(p2p_socket, &msg) < 0) {
        close(p2p_socket);
        return -1;
    }

    close(p2p_socket);

    return 0;
}
