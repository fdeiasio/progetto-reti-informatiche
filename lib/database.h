#ifndef DATABASE_H
#define DATABASE_H

#include "pch.h"

struct User {
    int fd;
    in_port_t port;
    
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
    int max_users;

    struct Lavagna lavagna;
};

struct DatabaseConfig {
    int max_users;

};

extern int database_init(struct DatabaseConfig config);

extern void database_cleanup();

extern int database_add_user(struct User* user);

extern int database_remove_user(in_port_t user_id);

extern int database_get_num_users();

extern int database_get_user_list(in_port_t* user_ports, int max_users);

extern int database_get_next_user_port();

extern void database_reset_user_iterator();

extern int database_get_port_from_fd(int fd);

extern void database_print_users();

extern int database_create_card(const char* text);

extern int database_card_doing(int card_id, in_port_t user_id);

extern int database_card_done(int card_id);

extern int database_card_todo(int card_id);

extern int database_get_next_todo_card();

extern int database_card_get_text(int card_id, char* buffer, size_t buffer_size);

extern void database_print_cards();
#endif // DATABASE_H