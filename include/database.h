#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"

// Questo header definisce le strutture che contengono tutti i dati del server
// Il database è implementato in modo tale che l'accesso possa essere effettuato 
// solo ed esclusivamente tramite funzioni in modo da garantirne la consistenza


enum UserStatus {
    USER_STATUS_ERROR,   // Stato non valido
    USER_STATUS_IDLE,    // L'utente non sta lavorando
    USER_STATUS_ACTIVE,  // L'utente ha una card assegnata
    USER_STATUS_PINGED,  // L'utente ha una card assegnata e il server sta aspettando un pong
};

// Definisce la struttura utente, implementata tramite linked list
struct User {
    int socket;
    in_port_t port;
    
    enum UserStatus status;
    int assigned_card_id;
    time_t last_ping_timestamp;

    struct User* next;
};

enum CardStatus {
    CARD_STATUS_ERROR,  // Stato non valido
    CARD_STATUS_TODO,   // La carta è nella lista to do
    CARD_STATUS_DOING,  // La carta è nella lista doing
    CARD_STATUS_DONE,   // La carta è nella lista done
};

// Definizione della struttura card, implementata tramita linked list
struct Card {
    int id;
    char text[256];

    enum CardStatus status;
    in_port_t assigned_user_port;
    time_t last_activity_timestamp;

    struct Card* next;
};

// Struttura lavagna che tiene le varie liste di card
struct Lavagna {
    int id;

    int num_cards;

    struct Card *todo;
    struct Card *doing;
    struct Card *done;
};

// Struttura database che tiene sia le carte che gli utenti
struct Database {
    struct User* users;
    int num_users;

    struct Lavagna lavagna;
};

// ======= FUNZIONI DATABASE =========

// Inizializza il db assegnando un id alla lavagna
extern void database_init();

// Si occupa di liberare tutta la memoria allocata al database
extern void database_cleanup();


// ======= FUNZIONI UTENTE ==========

// Aggiunge un utente al db attraverso il socket e il numero di porta
// Ritorna 0 se aggiunto con successo, -1 altrimenti
extern int database_add_user(int socket, in_port_t user_id);

// Rimuove l'utente attraverso il suo numero di porta
// Ritorna 0 se la rimozione è avvenuta con successo, -1 altrimenti
extern int database_remove_user(in_port_t user_id);

// Ritorna il numero di utenti attualmente connessi
extern int database_get_num_users();

// Allora un array dinamico contenente la tutti li id degli utenti connessi
// e lo mette nell'indirizzo puntato da user_ports
// Ritorna il numero di utenti attualmente connessi
extern int database_get_users_list(in_port_t** user_ports);

// Permette di accedere al id attraverso il socket
// Ritorna 0 se non esiste un utente con quel socket
extern in_port_t database_get_port_from_socket(int socket);

// Permette di accedere al socket a partire dal numero di porta
// Ritorna -1 se non esiste un utente con l'id corrispondente
extern int database_get_socket_from_port(in_port_t port);

// Permette di settare lo stato di un utente
// Ritorna 0 se lo stato è stato settato con successo, -1 se l'utente non esiste
extern int database_user_set_status(in_port_t user_id, enum UserStatus status);

// Permette di ottenere lo stato di un utente
// Ritorna USER_STATUS_ERROR se l'utente non esiste
extern enum UserStatus database_user_get_status(in_port_t user_id);

// Assegna una card ad un utente e imposta il suo stato ad ACTIVE
// Ritorna 0 se l'assegnazione è avvenuta con successo, -1 altrimenti
extern int database_user_assign_card(in_port_t user_id, int card_id);

// Setta il timestamp dell'ultimo ping e lo stato dell'utente a PINGED
// Ritorna -1 se l'utente non esiste
extern int database_user_set_ping(in_port_t user_id);

// Pulisce lo stato di ping di un utente, settandolo ad ACTIVE
// Ritorna -1 se l'utente non esiste
extern int database_user_clear_ping(in_port_t user_id);

// Iterasugli utenti per la funzione di assignment
// Ritorna il numero di porta, altrimenti -1
extern int database_get_next_user_port();

// Mette l'utente con user_id in pending
// Ritorna -1 se l'utente non esiste
extern int database_user_set_pending(int socket);

// Ritorna il socket dell'utente in pending, -1 se non c'è nessun utente in pending
extern int database_get_pending_user_socket();

// Rimuove lo stato di pending dall'utente
extern void database_user_clear_pending();

// Mette in user_ports un array dinamico con gli utenti in timeout
// Ritorna il numero di utenti che hanno superato il timeout
// -1 se non ci sono utenti in timeout
extern int database_user_get_timed_out(in_port_t** user_ports, int timeout);

// Utility che stampa tutti gli utenti connessi
extern void database_print_users();


// ======= FUNZIONI CARD ==========

// Crea una nuova card con il testo specificato e la aggiunge alla lista todo
// Ritorna l'id della card creata, -1 in caso di errore
extern int database_create_card(const char* text);

// Ottiene il testo della card con id specificato e lo copia nel buffer
// Ritorna la lunghezza del testo se l'operazione è avvenuta con successo, -1 in caso di errore
extern ssize_t database_card_get_text(int card_id, char* buffer, size_t buffer_size);

// Sposta la card assegnata all'utente dallo stato TODO a DOING
// Ritorna 0 se l'operazione è avvenuta con successo, -1 in caso di errore
extern int database_card_doing(in_port_t user_id);

// Sposta la card assegnata all'utente dallo stato DOING a DONE e mette l'utente in IDLE
// Ritorna 0 se l'operazione è avvenuta con successo, -1 in caso di errore
extern int database_card_done(in_port_t user_id);

// Sposta la card assegnata all'utente dallo stato DOING a TODO e mette l'utente in IDLE
// Ritorna 0 se l'operazione è avvenuta con successo, -1 in caso di errore
extern int database_card_todo(in_port_t user_id);

// Ritorna l'id dell'utente a cui è assegnata la card con id specificato
// Ritorna -1 se la card non esiste o non è assegnata
extern in_port_t database_card_get_user(int card_id);

// Setta l'ultima attività della card dell'utente all'ora corrente
// Ritorna -1 se l'utente non esiste o non ha una card assegnata, 0 altrimenti
extern int database_card_reset_timestamp(in_port_t user_id);

// Ottiene l'id della carta in testa alla todolist
// Torna -1 se è vuota
extern int database_get_next_todo_card();

// Mette in card_ids un array dinamico con le card in timeout nello stato DOING
// Ritorna il numero di card che hanno superato il timeout
// 0 se non ci sono card in timeout
extern int database_card_get_timed_out(int** card_ids, int timeout);

// Utility che stampa tutte le card suddivise per stato
extern void database_print_cards();

#endif // DATABASE_H