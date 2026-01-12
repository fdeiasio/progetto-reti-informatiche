#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "pch.h"

#define MAX_PAYLOAD_SIZE 256

/**
 * Questo header definisce il protocollo di comunicazione tra client e server.
 * Include la definizione dei tipi di messaggi, la struttura del messaggio e
 * le funzioni per inviare e ricevere messaggi. 
 */

/* Questo enum definisce il tipo dei messaggi che possono essere scambiati */
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

/* *
 * Questa struct tiene la struttura di un messaggio.
 *
 * type: Tipo del messaggio (definito nell'enum MessageType)
 * payload_length: Lunghezza del payload in byte
 * payload: Puntatore al payload del messaggio
 */
struct Message {
    uint32_t type;
    uint32_t payload_length;
    void* payload;
};

/* ==== Funzioni per inviare e ricevere messaggi ==== */

/* *
 * Invia il messaggio msg atraverso il socket socket.
 * Ritorna il numero di byte inviati, o -1 in caso di errore.
 */
ssize_t send_message(int socket, struct Message* msg);

/* *
 * Riceve un messaggio dal socket socket e lo memorizza in msg.
 * Ritorna il numero di byte ricevuti, o -1 in caso di errore.
 * Si assicura che msg->payload sia allocato con sufficiente spazio
 */
ssize_t receive_message(int socket, struct Message* msg);

/* *
 * Legge l'input da linea di comando e lo memorizza in msg.
 * Ritorna la lunghezza dell'input letto, o -1 in caso di errore.
 * 
 * La lunghezza massima dell'input è MAX_PAYLOAD_SIZE.
 */
ssize_t get_command_line_input(struct Message* msg);

#endif // PROTOCOL_H