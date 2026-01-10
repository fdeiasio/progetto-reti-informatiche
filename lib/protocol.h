#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "pch.h"

typedef enum {
    MSG_HELLO,
} MessageType;

typedef struct {
    MessageType type;
    uint32_t message;
} Message;

ssize_t send_message(int socket, Message msg);

ssize_t receive_message(int socket, Message* msg);

#endif // PROTOCOL_H