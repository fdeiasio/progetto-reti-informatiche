#include "pch.h"
#include "database.h"

Database* database_create(DatabaseConfig config) {
    Database* db = (Database*)malloc(sizeof(Database));
    if (!db) {
        fprintf(stderr, "Error: Could not allocate memory for database.\n");
        return NULL;
    }

    db->users = (User*)malloc(sizeof(User) * config.max_users);
    db->num_users = 0;
    db->max_users = config.max_users;

    return db;
}

void database_cleanup(Database* db) {
    if (db) {
        free(db->users);
        free(db);
    }
}

int database_add_user(Database* db, in_port_t user_id) {
    if (db->num_users >= db->max_users) {
        fprintf(stderr, "Error: Database is full. Cannot add more users.\n");
        return -1;
    }

    for (size_t i = 0; i < db->num_users; i++) {
        if (db->users[i].id == user_id) {
            fprintf(stderr, "Error: User with ID %d already exists.\n", user_id);
            return -1;
        }
    }

    db->users[db->num_users].id = user_id;
    db->num_users++;

    return 0;
}