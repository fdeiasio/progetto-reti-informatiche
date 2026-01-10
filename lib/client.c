#include "pch.h"
#include "protocol.h"
#include "client.h"

Client* client_create(in_port_t port) {
    Client* client = (Client*)malloc(sizeof(Client));
    if (!client) {
        fprintf(stderr, "Error: Could not allocate memory for client.\n");
        exit(1);
    }

    client->port = port;
    client->state = STATE_STARTING_P2P;
    pthread_mutex_init(&client->state_mutex, NULL);
    client->server_socket = -1;

    return client;
}

void client_update_state(Client* client, ClientState new_state) {
    pthread_mutex_lock(&client->state_mutex);
    client->state = new_state;
    pthread_mutex_unlock(&client->state_mutex);
}

void client_start_p2p(Client* client, void* p2p_server_function(void*)) {
    pthread_create(&client->p2p_server_thread, NULL, p2p_server_function, client);
}

void client_connect_to_server(Client* client, const char* server_ip, in_port_t server_port) {
    client->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client->server_socket < 0) {
        fprintf(stderr, "Error: Could not create socket.\n");
        exit(1);
    }

    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &client->server_addr.sin_addr);

    if (connect(client->server_socket, (struct sockaddr *)&client->server_addr, sizeof(client->server_addr)) < 0) {
        fprintf(stderr, "Error: Could not connect to server.\n");
        close(client->server_socket);
        exit(1);
    }

    fprintf(stdout, "Connected to server\n");
}

void client_send_server_hello(Client* client) {
    Message msg = {
        .type = MSG_HELLO,
        .message = client->port,
    };

    if (send_message(client->server_socket, msg) < 0) {
        fprintf(stderr, "Error: Could not send HELLO message to server.\n");
    } else {
        fprintf(stdout, "Sent HELLO message to server\n");
    }
}