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
        .todo = NULL,
        .doing = NULL,
        .done = NULL,
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

int database_get_num_users() {
    struct Database* db = &database;
    return db->num_users;
}

void database_get_user_list(in_port_t* user_ports, int max_users) {
    struct Database* db = &database;

    int count = (db->num_users < max_users) ? db->num_users : max_users;
    for (int i = 0; i < count; i++) {
        user_ports[i] = db->users[i].port;
    }
}

int database_get_next_user_port() {
    struct Database* db = &database;
    static in_port_t last_chosen = 0;

    if (db->num_users == 0) {
        return -1;
    }

    int min_port_index = 0;
    for (int i = 1; i < db->num_users; i++) {
        if (db->users[i].port < db->users[min_port_index].port) {
            min_port_index = i;
        }
    }

    if (last_chosen == 0) {
        last_chosen = db->users[min_port_index].port;
        return last_chosen;
    }

    in_port_t next_port = 0;
    int found = 0;
    for (int i = 0; i < db->num_users; i++) {
        if (db->users[i].port > last_chosen) {
            if (!found || db->users[i].port < next_port) {
                next_port = db->users[i].port;
                found = 1;
            }
        }
    }

    if (!found) {
        next_port = db->users[min_port_index].port;
    }

    last_chosen = next_port;

    return last_chosen;
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
    new_card->status = CARD_STATUS_TODO; 
    strncpy(new_card->text, text, sizeof(new_card->text) - 1);
    new_card->text[sizeof(new_card->text) - 1] = '\0';
    new_card->user = 0;
    new_card->timestamp = 0;
    new_card->next = NULL;

    if (db->lavagna.todo == NULL) {
        db->lavagna.todo = new_card;
    } 
    else {
        struct Card* current = db->lavagna.todo;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_card;
    }   
    
    db->lavagna.num_cards++;

    return new_card->id;
}

void database_card_doing(int card_id, in_port_t user_id) {
    struct Database* db = &database;

    for (struct Card** curr = &db->lavagna.todo; *curr != NULL; curr = &(*curr)->next) {
        if ((*curr)->id == card_id) {
            struct Card* card = *curr;
            *curr = card->next;

            card->status = CARD_STATUS_DOING;
            card->user = user_id;
            if (db->lavagna.doing == NULL) {
                db->lavagna.doing = card;
            } 
            else {
                struct Card* current = db->lavagna.doing;
                while (current->next != NULL) {
                    current = current->next;
                }
                current->next = card;
            }   

            return;
        }
    }   
}

void database_card_done(int card_id) {
    struct Database* db = &database;

    for (struct Card** curr = &db->lavagna.doing; *curr != NULL; curr = &(*curr)->next) {
        if ((*curr)->id == card_id) {
            struct Card* card = *curr;
            *curr = card->next;

            card->status = CARD_STATUS_DONE;
            if (db->lavagna.done == NULL) {
                db->lavagna.done = card;
            } 
            else {
                struct Card* current = db->lavagna.done;
                while (current->next != NULL) {
                    current = current->next;
                }
                current->next = card;
            }   

            return;
        }
    }
}

void database_card_todo(int card_id) {
    struct Database* db = &database;

    for (struct Card** curr = &db->lavagna.doing; *curr != NULL; curr = &(*curr)->next) {
        if ((*curr)->id == card_id) {
            struct Card* card = *curr;
            *curr = card->next;

            card->status = CARD_STATUS_TODO;
            card->user = 0;
            if (db->lavagna.todo == NULL) {
                db->lavagna.todo = card;
            } 
            else {
                struct Card* current = db->lavagna.todo;
                while (current->next != NULL) {
                    current = current->next;
                }
                current->next = card;
            }   

            return;
        }
    }   
}

int database_get_next_todo_card() {
    struct Database* db = &database;

    if (db->lavagna.todo == NULL) {
        return -1;
    }

    return db->lavagna.todo->id;
}

void database_card_get_text(int card_id, char* buffer, size_t buffer_size) {
    struct Database* db = &database;

    for (struct Card* current = db->lavagna.todo; current != NULL; current = current->next) {
        if (current->id == card_id) {
            strncpy(buffer, current->text, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            return;
        }
    }

    for (struct Card* current = db->lavagna.doing; current != NULL; current = current->next) {
        if (current->id == card_id) {
            strncpy(buffer, current->text, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            return;
        }
    }

    for (struct Card* current = db->lavagna.done; current != NULL; current = current->next) {
        if (current->id == card_id) {
            strncpy(buffer, current->text, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            return;
        }
    }

    buffer[0] = '\0'; 
}
void database_print_cards() {
    struct Database* db = &database;

    fprintf(stdout, "==================== Current Cards in Lavagna ====================\n");

    struct Card* current = db->lavagna.todo;
    fprintf(stdout, "-- TODO --\n");
    while (current) {
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->user, current->text);
        current = current->next;
    }

    current = db->lavagna.doing;
    fprintf(stdout, "-- DOING --\n");
    while (current) {
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->user, current->text);
        current = current->next;
    }

    current = db->lavagna.done;
    fprintf(stdout, "-- DONE --\n");
    while (current) {
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->user, current->text);
        current = current->next;
    }

    fprintf(stdout, "=================================================================\n");
}