#include "pch.h"
#include "client.h"

struct Client* client_create(struct ClientConfig config) {
    struct Client* client = (struct Client*)malloc(sizeof(struct Client));
    if (!client) {
        return NULL;
    }

    client->port = config.client_port;

    client->state = STATE_STARTING_P2P;
    pthread_mutex_init(&client->state_mutex, NULL);

    client->server_socket = -1;
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(config.server_port);
    client->server_addr.sin_addr.s_addr = config.server_ip;

    FD_ZERO(&client->master_set);
    FD_ZERO(&client->read_fds);

    FD_SET(STDIN_FILENO, &client->master_set);
    client->max_fd = STDIN_FILENO;

    client->server_handler = config.server_handler;
    client->stdin_handler = config.stdin_handler;

    return client;
}

void client_update_state(struct Client* client, enum ClientState new_state) {
    pthread_mutex_lock(&client->state_mutex);
    client->state = new_state;
    pthread_mutex_unlock(&client->state_mutex);
}

void client_start_p2p(struct Client* client, void* p2p_server_function(void*)) {
    if (pthread_create(&client->p2p_server_thread, NULL, p2p_server_function, client) != 0) {
        fprintf(stderr, "Error: Could not create P2P server thread.\n");
        exit(1);
    }
}

void client_stop_p2p(struct Client* client) {
    client_update_state(client, STATE_DISCONNECTING);
    pthread_join(client->p2p_server_thread, NULL);
}

void client_connect_to_server(struct Client* client) {
    fprintf(stdout, "Connecting to server...\n");

    client->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client->server_socket < 0) {
        fprintf(stderr, "Error: Could not create socket.\n");
        exit(1);
    }

    if (connect(client->server_socket, (struct sockaddr *)&client->server_addr, sizeof(client->server_addr)) < 0) {
        fprintf(stderr, "Error: Could not connect to server.\n");
        close(client->server_socket);
        exit(1);
    }

    FD_SET(client->server_socket, &client->master_set);
    if (client->server_socket > client->max_fd) {
        client->max_fd = client->server_socket;
    }
}

void client_disconnect_from_server(struct Client* client) {
    if (client->server_socket != -1) {
        close(client->server_socket);
        FD_CLR(client->server_socket, &client->master_set);
        client->server_socket = -1;
    }
}

void client_destroy(struct Client* client) {
    if (client) {
        pthread_mutex_destroy(&client->state_mutex);
        free(client);
    }
}

int client_listen(struct Client* client) {
    client->read_fds = client->master_set;

    struct timeval timeout = {
        .tv_sec = 0,
        .tv_usec = 100000,
    };

    int activity = select(client->max_fd + 1, &client->read_fds, NULL, NULL, &timeout);
    if (activity < 0) {
        if (errno == EINTR) {
            return -1;
        }

        fprintf(stderr, "Error: select() failed.\n");
        return -1;
    }

    if (FD_ISSET(STDIN_FILENO, &client->read_fds)) {
        if (client->stdin_handler && client->stdin_handler(client) < 0) {
            return -1;
        }
    }

    if (FD_ISSET(client->server_socket, &client->read_fds)) {
        if (client->server_handler && client->server_handler(client) < 0) {
            return -1;
        }
    }

    return 0;
}
