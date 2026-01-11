#include "pch.h"
#include "database.h"

struct Database database;

int database_init(struct DatabaseConfig config) {
    struct Database* db = &database;

    db->users = (struct User*)malloc(sizeof(struct User) * config.max_users);
    if (!db->users) {
        fprintf(stderr, "Error: Could not allocate memory for users.\n");
        return -1;
    }
    db->num_users = 0;
    db->max_users = config.max_users;

    db->lavagna = (struct Lavagna) {
        .id = 1,
        .num_cards = 0,
        .todo_head = NULL,
        .todo_tail = NULL,
        .doing_head = NULL,
        .doing_tail = NULL,
        .done_head = NULL,
        .done_tail = NULL,
    };

    return 0;
}

void database_cleanup() {
    struct Database* db = &database;

    free(db->users);
}

int database_get_user_from_port(in_port_t user_id) {
    struct Database* db = &database;

    for (int i = 0; i < db->num_users; i++) {
        if (db->users[i].port == user_id) {
            return i;
        }
    }

    return -1; 
}

int database_get_port_from_fd(int fd) {
    struct Database* db = &database;

    for (int i = 0; i < db->num_users; i++) {
        if (db->users[i].fd == fd) {
            return db->users[i].port;
        }
    }

    return -1; 
}

int database_add_user(struct User* user) {
    struct Database* db = &database;

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
    struct Database* db = &database;

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
    struct Database* db = &database;

    fprintf(stdout, "==== Current Users in Database ====\n");
    for (int i = 0; i < db->num_users; i++) {
        fprintf(stdout, "> User %d: Port=%d\n", i + 1, db->users[i].port);
    }
    fprintf(stdout, "===================================\n");
}

int database_create_card(in_port_t user_id, const char* text)  {
    struct Database* db = &database;

    struct Card* new_card = (struct Card*)malloc(sizeof(struct Card));
    if (!new_card) {
        fprintf(stderr, "Error: Could not allocate memory for new card.\n");
        return -1;
    }

    new_card->id = db->lavagna.num_cards + 1;
    new_card->status = 0; 
    strncpy(new_card->text, text, sizeof(new_card->text) - 1);
    new_card->text[sizeof(new_card->text) - 1] = '\0';
    new_card->user = user_id;
    new_card->timestamp = 0;
    new_card->next = NULL;

    if (db->lavagna.todo_tail) {
        db->lavagna.todo_tail->next = new_card;
        db->lavagna.todo_tail = new_card;
    } else {
        db->lavagna.todo_head = new_card;
        db->lavagna.todo_tail = new_card;
    }

    db->lavagna.num_cards++;

    return new_card->id;
}

void database_print_cards() {
    struct Database* db = &database;

    fprintf(stdout, "==================== Current Cards in Lavagna ====================\n");

    struct Card* current = db->lavagna.todo_head;
    fprintf(stdout, "-- TODO --\n");
    while (current) {
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->user, current->text);
        current = current->next;
    }

    current = db->lavagna.doing_head;
    fprintf(stdout, "-- DOING --\n");
    while (current) {
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->user, current->text);
        current = current->next;
    }

    current = db->lavagna.done_head;
    fprintf(stdout, "-- DONE --\n");
    while (current) {
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->user, current->text);
        current = current->next;
    }

    fprintf(stdout, "=================================================================\n");
}