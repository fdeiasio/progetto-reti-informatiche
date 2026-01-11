#include "pch.h"
#include "protocol.h"

const char *MESSAGE_STRINGS[NUM_MSG_TYPES] = {
    [MSG_HELLO] = "HELLO",
    [MSG_QUIT] = "QUIT",
    [MSG_SHOW_UTENTI] = "SHOW_UTENTI",
    [MSG_SHOW_LAVAGNA] = "SHOW_LAVAGNA",
    [MSG_CREATE_CARD] = "CREATE_CARD",
    [MSG_SEND_USER_LIST] = "SEND_USER_LIST",
    [MSG_REQUEST_USER_LIST] = "REQUEST_USER_LIST",
    [MSG_HANDLE_CARD] = "HANDLE_CARD",
    [MSG_ACK_CARD] = "ACK_CARD",

};

ssize_t send_message(int socket, struct Message* msg) {
    uint32_t buffer[2];

    buffer[0] = htonl(msg->type);
    buffer[1] = htonl(msg->payload_length);

    ssize_t bytes_sent = send(socket, buffer, sizeof(buffer), 0);
    if (bytes_sent <= 0) {
        return bytes_sent;
    }

    if (!msg->payload_length) {
        return bytes_sent;
    }

    if (!msg->payload) {
        fprintf(stderr, "Error: Payload is NULL.\n");
        return -1;
    }

    bytes_sent = send(socket, msg->payload, msg->payload_length, 0);
    if (bytes_sent <= 0) {
        return bytes_sent;
    }

    return bytes_sent + sizeof(buffer);
}

ssize_t receive_message(int socket, struct Message* msg) {
    uint32_t buffer[2];
    ssize_t bytes_received = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        return bytes_received;
    }

    msg->type = ntohl(buffer[0]);
    msg->payload_length = ntohl(buffer[1]);

    if (!msg->payload_length) {
        return bytes_received;
    }

    if (!msg->payload) {
        fprintf(stderr, "Error: Payload buffer is NULL.\n");
        return -1;
    }

    bytes_received = recv(socket, msg->payload, msg->payload_length, 0);
    if (bytes_received <= 0) {
        return bytes_received;
    }

    return bytes_received + sizeof(buffer);
}

ssize_t get_command_line_input(struct Message* msg) {
    char buffer[MAX_PAYLOAD_SIZE + 32];
    ssize_t input_length = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
    if (input_length < 0) {
        fprintf(stderr, "Error: Could not read from stdin.\n");
        return -1;
    }

    // Remove trailing newline
    buffer[input_length - 1] = '\0';
    
    msg->type = MSG_ERR;
    for (int i = 1; i < NUM_MSG_TYPES; i++) {
        if (strncmp(buffer, MESSAGE_STRINGS[i], strlen(MESSAGE_STRINGS[i])) == 0) {
            msg->type = i;
        }
    }

    if (msg->type == MSG_ERR) {
        fprintf(stderr, "Error: Unknown command.\n");
        return -1;
    }

    int payload_start = strlen(MESSAGE_STRINGS[msg->type]);
    while (buffer[payload_start] == ' ') {
        payload_start++;
    }

    msg->payload_length = input_length - payload_start - 1;
    if (msg->payload_length > 0) {
        memcpy(msg->payload, &buffer[payload_start], msg->payload_length);
    } 

    return input_length;
}