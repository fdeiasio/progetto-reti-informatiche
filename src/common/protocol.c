#include "../../include/common.h"
#include "../../include/protocol.h"
 
/* ==== Costanti e Tabelle di Supporto ==== */
const char *MESSAGE_TO_STRING[NUM_MSG_TYPES] = {
    [MSG_HELLO] = "HELLO",
    [MSG_QUIT] = "QUIT",
    [MSG_SHOW_UTENTI] = "SHOW_UTENTI",
    [MSG_SHOW_LAVAGNA] = "SHOW_LAVAGNA",
    [MSG_CREATE_CARD] = "CREATE_CARD",
    [MSG_SEND_USER_LIST] = "SEND_USER_LIST",
    [MSG_REQUEST_USER_LIST] = "REQUEST_USER_LIST",
    [MSG_HANDLE_CARD] = "HANDLE_CARD",
    [MSG_ACK_CARD] = "ACK_CARD",
    [MSG_REVIEW_CARD] = "REVIEW_CARD",
    [MSG_PING_USER] = "PING_USER",
    [MSG_PONG_LAVAGNA] = "PONG_LAVAGNA",
    [MSG_CARD_DONE] = "CARD_DONE",
    [MSG_DONE_REVIEW] = "DONE_REVIEW",
};


ssize_t send_message(int socket, struct Message* msg) {

    // Creo un buffer temporaneo per modificare l'endianness
    uint32_t buffer[2];
    buffer[0] = htonl(msg->type);
    buffer[1] = htonl(msg->payload_length);

    // Invio prima il tipo e la lunghezza del payload
    ssize_t bytes_sent = send(socket, buffer, sizeof(buffer), 0);
    if (bytes_sent <= 0) {
        return bytes_sent;
    }

    // Se non c'è payload, termino qui
    if (!msg->payload_length) {
        return bytes_sent;
    }

    // Mi assicuro che il payload non sia NULL
    if (!msg->payload) {
        return -1;
    }

    // Invio il payload
    bytes_sent = send(socket, msg->payload, msg->payload_length, 0);
    if (bytes_sent <= 0) {
        return bytes_sent;
    }

    return bytes_sent + sizeof(buffer);
}

ssize_t receive_message(int socket, struct Message* msg) {

    // Creo un buffer temporaneo per leggere il tipo e la lunghezza
    uint32_t buffer[2];
    ssize_t bytes_received = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        return bytes_received;
    }

    msg->type = ntohl(buffer[0]);
    msg->payload_length = ntohl(buffer[1]);

    // Se non c'è payload, termino qui
    if (!msg->payload_length) {
        return bytes_received;
    }

    // Mi assicuro che il payload sia allocato con sufficiente spazio
    if (!msg->payload) {
        return -1;
    }

    // Ricevo il payload
    bytes_received = recv(socket, msg->payload, msg->payload_length, 0);
    if (bytes_received <= 0) {
        return bytes_received;
    }

    return bytes_received + sizeof(buffer);
}

// Funzione di utility per pulire il buffer
static void clear_stdin_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

ssize_t get_command_line_input(struct Message* msg) {
    // Leggo l'input da stdin
    char buffer[MAX_PAYLOAD_SIZE];
    ssize_t input_length = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
    if (input_length < 0) {
        return -1;
    }
    
    if (buffer[input_length - 1] != '\n') {
        clear_stdin_buffer();
        return -1;
    }
    buffer[input_length - 1] = '\0';
    
    // Identifico il tipo di messaggio
    msg->type = MSG_ERR;
    for (int i = 1; i < NUM_MSG_TYPES; i++) {
        if (strncmp(buffer, MESSAGE_TO_STRING[i], strlen(MESSAGE_TO_STRING[i])) == 0) {
            msg->type = i;
        }
    }
    if (msg->type == MSG_ERR) {
        fprintf(stderr, "Error: Unknown command.\n");
        return -1;
    }

    // Estraggo il payload (se presente)
    int payload_start = strlen(MESSAGE_TO_STRING[msg->type]);
    while (buffer[payload_start] == ' ') {
        payload_start++;
    }

    msg->payload_length = input_length - payload_start - 1;
    if (msg->payload_length > 0) {
        memcpy(msg->payload, &buffer[payload_start], msg->payload_length);
    } 

    return input_length;
}