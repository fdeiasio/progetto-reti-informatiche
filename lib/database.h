#ifndef DATABASE_H
#define DATABASE_H

#include "pch.h"

typedef struct {
    int fd;
    in_port_t port;
    
} User;

typedef struct {
    int id;

} Card;

typedef struct Database {
    User* users;
    int num_users;
    int max_users;

} Database;

typedef struct {
    int max_users;

} DatabaseConfig;

extern int database_init(DatabaseConfig config);

extern void database_cleanup();

extern int database_add_user(User* user);

extern int database_remove_user(in_port_t user_id);

extern int database_get_user_from_port(in_port_t user_id);

extern int database_get_port_from_fd(int fd);

extern void database_print_users();
#endif // DATABASE_H