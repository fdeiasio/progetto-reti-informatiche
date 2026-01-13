#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "common.h"

#define MAX_PAYLOAD_SIZE 256

// Questo header definisce il protocollo di comunicazione tra client e server.

enum MessageType{
    MSG_ERR,                // Messaggio di errore, non può essere inviato   
     
    MSG_HELLO,              // Viene mandato dal client per registrarsi al server
    MSG_QUIT,               // Viene mandato dal client per annunciare la disconnessione

    MSG_SHOW_UTENTI,        // Viene mandato dal terminale del server per mostrare gli utenti connessi
    MSG_SHOW_LAVAGNA,       // Viene mandato dal terminale del server per mostrare lo stato della lavagna

    MSG_CREATE_CARD,        // Viene mandato dal client per richiedere la creazione di una nuova card
    MSG_HANDLE_CARD,        // Viene mandato dal server per assegnare una card al client
    MSG_ACK_CARD,           // Viene mandato dal client per confermare l'assegnazione della card
    MSG_CARD_DONE,          // Viene mandato dal client per segnalare il completamento della card

    MSG_SEND_USER_LIST,     // Viene mandato dal server per inviare la lista degli utenti connessi
    MSG_REQUEST_USER_LIST,  // Viene mandato dal client per richiedere la lista degli utenti connessi
   
    MSG_REVIEW_CARD,        // Viene mandato da un peer per richiedere la revisione di una card
    MSG_DONE_REVIEW,        // Viene mandato da un peer per segnalare il completamento della revisione di una card

    MSG_PING_USER,          // Viene mandato dal server per verificare la raggiungibilità di un client
    MSG_PONG_LAVAGNA,       // Viene mandato dal client in risposta al PING_USER
    

    NUM_MSG_TYPES,          // Numero totale di tipi di messaggi
};


// Struttua che tiene un messaggio
struct Message {
    uint32_t type;
    uint32_t payload_length;
    void* payload;
};

// Invia un messaggio attraverso il socket
// Ritorna il numero di byte inviati, o -1 in caso di errore.
ssize_t send_message(int socket, struct Message* msg);

// Riceve un messaggio dal socket
// Ritorna il numero di byte ricevuti, o -1 in caso di errore
ssize_t receive_message(int socket, struct Message* msg);

// Legge l'input da linea di comando e lo converte in un messaggio
// Ritorna il numero di byte letti, o -1 in caso di errore
ssize_t get_command_line_input(struct Message* msg);

#endif // PROTOCOL_H