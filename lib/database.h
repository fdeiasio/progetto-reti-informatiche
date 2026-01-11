#ifndef DATABASE_H
#define DATABASE_H

#include "pch.h"

struct User {
    int fd;
    in_port_t port;
    
};

struct Card {
    int id;
    int status;
    char text[256];
    int user;
    uint32_t timestamp;

    struct Card* next;
};

struct Lavagna {
    int id;

    int num_cards;

    struct Card *todo_head;
    struct Card *todo_tail;
    struct Card *doing_head;
    struct Card *doing_tail;
    struct Card *done_head;
    struct Card *done_tail;
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

extern int database_get_user_from_port(in_port_t user_id);

extern int database_get_port_from_fd(int fd);

extern void database_print_users();

extern int database_create_card(in_port_t user_id, const char* text);

extern void database_print_cards();
#endif // DATABASE_H