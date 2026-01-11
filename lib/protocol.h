#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "pch.h"

#define MAX_PAYLOAD_SIZE 256

enum MessageType{
    MSG_ERR,
    MSG_HELLO,
    MSG_QUIT,
    MSG_SHOW_UTENTI,
    MSG_SHOW_LAVAGNA,
    MSG_CREATE_CARD,
    MSG_SEND_USER_LIST,
    MSG_REQUEST_USER_LIST,
    MSG_HANDLE_CARD,
    MSG_ACK_CARD,

    NUM_MSG_TYPES,
};

struct Message {
    uint32_t type;
    uint32_t payload_length;
    void* payload;
};
ssize_t send_message(int socket, struct Message* msg);

ssize_t receive_message(int socket, struct Message* msg);

ssize_t get_command_line_input(struct Message* msg);

#endif // PROTOCOL_H