#include "pch.h"
#include "messaging.h"

ssize_t send_message(int socket, Message msg) {
    uint32_t buffer[2];
    buffer[0] = htonl((uint32_t)msg.type);
    buffer[1] = htonl(msg.message);

    ssize_t bytes_sent = send(socket, buffer, sizeof(buffer), 0);
    return bytes_sent;
}

ssize_t receive_message(int socket, Message* msg) {
    uint32_t buffer[2];
    ssize_t bytes_received = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        return bytes_received;
    }

    msg->type = (MessageType) ntohl(buffer[0]);
    msg->message = ntohl(buffer[1]);

    return bytes_received;
}