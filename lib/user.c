#include "pch.h"
#include "messaging.h"
#include "user.h"

User* user_create(in_port_t port) {
    User* user = (User*)malloc(sizeof(User));
    if (!user) {
        fprintf(stderr, "Error: Could not allocate memory for user.\n");
        exit(1);
    }

    user->port = port;
    user->state = STATE_CONNECTING;
    user->server_socket = -1;

    return user;
}

void user_start_p2p(User* user, void* p2p_server_function(void*)) {
    pthread_create(&user->p2p_server_thread, NULL, p2p_server_function, user);
}

void user_connect_to_server(User* user, const char* server_ip, in_port_t server_port) {
    user->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (user->server_socket < 0) {
        fprintf(stderr, "Error: Could not create socket.\n");
        exit(1);
    }

    user->server_addr.sin_family = AF_INET;
    user->server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &user->server_addr.sin_addr);

    if (connect(user->server_socket, (struct sockaddr *)&user->server_addr, sizeof(user->server_addr)) < 0) {
        fprintf(stderr, "Error: Could not connect to server.\n");
        close(user->server_socket);
        exit(1);
    }

    fprintf(stdout, "Connected to server\n");
}

void user_send_server_hello(User* user) {
    Message msg = {
        .type = MSG_HELLO,
        .message = 0,
    };

    if (send_message(user->server_socket, msg) < 0) {
        fprintf(stderr, "Error: Could not send hello message to server.\n");
    } else {
        fprintf(stdout, "Sent hello message to server\n");
    }
}