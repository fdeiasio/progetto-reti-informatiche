#include "pch.h"
#include "database.h"

Database database;

int database_init(DatabaseConfig config) {
    Database* db = &database;

    db->users = (User*)malloc(sizeof(User) * config.max_users);
    if (!db->users) {
        fprintf(stderr, "Error: Could not allocate memory for users.\n");
        return -1;
    }
    db->num_users = 0;
    db->max_users = config.max_users;

    return 0;
}

void database_cleanup() {
    Database* db = &database;

    free(db->users);
}

int database_get_user_from_port(in_port_t user_id) {
    Database* db = &database;

    for (int i = 0; i < db->num_users; i++) {
        if (db->users[i].port == user_id) {
            return i;
        }
    }

    return -1; 
}

int database_get_port_from_fd(int fd) {
    Database* db = &database;

    for (int i = 0; i < db->num_users; i++) {
        if (db->users[i].fd == fd) {
            return db->users[i].port;
        }
    }

    return -1; 
}


int database_add_user(User* user) {
    Database* db = &database;

    if (db->num_users >= db->max_users) {
        fprintf(stderr, "Error: Database is full. Cannot add more users.\n");
        return -1;
    }

    if (database_get_user_from_port(user->port) != -1) {
        fprintf(stderr, "Error: User with ID %d already exists.\n", user->port);
        return -1;
    }

    db->users[db->num_users].fd = user->fd;
    db->users[db->num_users].port = user->port;
    db->num_users++;

    return 0;
}

int database_remove_user(in_port_t user_id) {
    Database* db = &database;

    int index = database_get_user_from_port(user_id);
    if (index == -1) {
        return -1;
    }

    for (int i = index; i < db->num_users - 1; i++) {
        db->users[i] = db->users[i + 1];
    }
    db->num_users--;

    return 0;
}

void database_print_users() {
    Database* db = &database;

    fprintf(stdout, "==== Current Users in Database ====\n");
    for (int i = 0; i < db->num_users; i++) {
        fprintf(stdout, "> User %d: Port=%d\n", i + 1, db->users[i].port);
    }
    fprintf(stdout, "===================================\n");
}