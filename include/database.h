#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"

enum UserStatus {
    USER_STATE_IDLE,
    USER_STATE_WAITING_ACK,
    USER_STATE_WORKING,
};

struct User {
    int fd;
    in_port_t port;
    
    enum UserStatus status;
    int card_id;

    struct User* next;
};

enum CardStatus {
    CARD_STATUS_TODO = 0,
    CARD_STATUS_DOING = 1,
    CARD_STATUS_DONE = 2,
};

struct Card {
    int id;
    enum CardStatus status;
    char text[256];
    int user;
    uint32_t timestamp;

    struct Card* next;
};

struct Lavagna {
    int id;

    int num_cards;

    struct Card *todo;
    struct Card *doing;
    struct Card *done;
};

struct Database {
    struct User* users;
    int num_users;

    struct Lavagna lavagna;
};

/* ===== Database Functions ===== */
extern void database_init();

extern void database_cleanup();

/*===== Database Users Functions =====*/
extern int database_add_user(int fd, in_port_t port);

extern int database_remove_user(in_port_t user_id);

extern int database_get_num_users();

extern int database_get_user_list(in_port_t** user_ports);

extern int database_get_next_user_port();

extern void database_reset_user_iterator();

extern int database_get_port_from_fd(int fd);

extern int database_get_fd_from_port(in_port_t port);

extern void database_user_set_status(in_port_t user_id, enum UserStatus status);

extern void database_user_assign_card(in_port_t user_id, int card_id);

extern void database_print_users();

/*===== Database Cards Functions =====*/
extern int database_create_card(const char* text);

extern int database_card_doing(in_port_t user_id);

extern int database_card_done(in_port_t user_id);

extern int database_card_todo(in_port_t user_id);

extern int database_get_next_todo_card();

extern int database_card_get_text(int card_id, char* buffer, size_t buffer_size);

extern void database_print_cards();

#endif // DATABASE_H