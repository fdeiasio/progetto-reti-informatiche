#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "pch.h"

#define MAX_PAYLOAD_SIZE 256

typedef enum {
    MSG_HELLO,
    MSG_QUIT,

    NUM_MSG_TYPES,
} MessageType;

typedef struct {
    uint32_t type;
    uint32_t payload_length;
    void* payload;
} Message;

ssize_t send_message(int socket, Message* msg);

ssize_t receive_message(int socket, Message* msg);

ssize_t get_command_line_input(Message* msg);

#endif // PROTOCOL_H