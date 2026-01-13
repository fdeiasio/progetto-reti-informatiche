#include "../../include/common.h"
#include "../../include/client.h"

struct Client* client_create(struct ClientConfig config) {
    struct Client* client = (struct Client*)malloc(sizeof(struct Client));
    if (!client) {
        return NULL;
    }

    client->server_socket = -1;
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(config.server_port);
    client->server_addr.sin_addr.s_addr = config.server_ip;

    FD_ZERO(&client->master_set);
    FD_SET(STDIN_FILENO, &client->master_set);
    client->max_fd = STDIN_FILENO;

    client->server_message_callback = config.server_message_callback;
    client->stdin_message_callback = config.stdin_message_callback;

    return client;
}

int client_connect_to_server(struct Client* client) {
    client->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client->server_socket < 0) {
        return -1;
    }

    if (connect(client->server_socket, (struct sockaddr *)&client->server_addr, sizeof(client->server_addr)) < 0) {
        close(client->server_socket);
        client->server_socket = -1;
        return -1;
    }

    // Aggiungo il server ai descrittori da monitorare
    FD_SET(client->server_socket, &client->master_set);
    if (client->server_socket > client->max_fd) {
        client->max_fd = client->server_socket;
    }

    return 0;
}

void client_destroy(struct Client* client) {
    // Chiudo la connessione al server prima di liberare il client
    if (client->server_socket != -1) {
        FD_CLR(client->server_socket, &client->master_set);
        close(client->server_socket);
        client->server_socket = -1;
    }

    if (client) {
        free(client);
    }
}

int client_listen(struct Client* client) {
    fd_set read_fds = client->master_set;

    struct timeval timeout = {
        .tv_sec = 0,
        .tv_usec = 100000,
    };

    int activity = select(client->max_fd + 1, &read_fds, NULL, NULL, &timeout);
    if (activity < 0) {
        return -1;
    }

    // Chiamo le callback per stdin e server
    // Controllo che non siano NULL prima di accerede
    // Passo il server_fd per permettere la comunicazione col server
    // Se ritornano -1 richiedo la terminazione del client
    int server_fd = client->server_socket;

    if (FD_ISSET(STDIN_FILENO, &read_fds)) {
        if (client->stdin_message_callback && client->stdin_message_callback(&server_fd) < 0) {
            return -1;
        }
    }

    if (FD_ISSET(client->server_socket, &read_fds)) {
        if (client->server_message_callback && client->server_message_callback(&server_fd) < 0) {
            return -1;
        }
    }

    return 0;
}
