#ifndef DATABASE_H
#define DATABASE_H

#include "pch.h"

typedef struct {
    int id;

} User;

typedef struct {
    int id;

} Card;

typedef struct Database {
    User* users;
    size_t num_users;
    size_t max_users;

} Database;

typedef struct {
    size_t max_users;

} DatabaseConfig;

extern Database* database_create(DatabaseConfig config);

extern void database_cleanup(Database* db);

extern int database_add_user(Database* db, in_port_t user_id);

#endif // DATABASE_H