#include "../../include/common.h"
#include "../../include/database.h"

struct Database database;

/* ==================== Static Helper Functions ==================== */

static void free_card_list(struct Card* head) {
    while (head) {
        struct Card* next = head->next;
        free(head);
        head = next;
    }
}

static void append_card(struct Card** head, struct Card* card) {
    card->next = NULL;
    if (*head == NULL) {
        *head = card;
    } else {
        struct Card* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = card;
    }
}

static struct Card* remove_card_from_list(struct Card** head, int card_id) {
    for (struct Card** curr = head; *curr != NULL; curr = &(*curr)->next) {
        if ((*curr)->id == card_id) {
            struct Card* card = *curr;
            *curr = card->next;
            card->next = NULL;
            return card;
        }
    }
    return NULL;
}

static struct Card* find_card_in_list(struct Card* head, int card_id) {
    for (struct Card* curr = head; curr != NULL; curr = curr->next) {
        if (curr->id == card_id) {
            return curr;
        }
    }
    return NULL;
}

static struct Card* database_find_card(int card_id) {
    struct Database* db = &database;
    struct Card* card;

    card = find_card_in_list(db->lavagna.todo, card_id);
    if (card) return card;

    card = find_card_in_list(db->lavagna.doing, card_id);
    if (card) return card;

    card = find_card_in_list(db->lavagna.done, card_id);
    return card;
}

static int database_get_user_from_port(in_port_t user_id) {
    struct Database* db = &database;

    for (int i = 0; i < db->num_users; i++) {
        if (db->users[i].port == user_id) {
            return i;
        }
    }

    return -1; 
}


/* ==================== Database Initialization ==================== */

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

    free_card_list(db->lavagna.todo);
    free_card_list(db->lavagna.doing);
    free_card_list(db->lavagna.done);

    db->lavagna.todo = NULL;
    db->lavagna.doing = NULL;
    db->lavagna.done = NULL;
    db->lavagna.num_cards = 0;

    free(db->users);
    db->users = NULL;
    db->num_users = 0;
    db->max_users = 0;
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

int database_get_fd_from_port(in_port_t port) {
    struct Database* db = &database;

    for (int i = 0; i < db->num_users; i++) {
        if (db->users[i].port == port) {
            return db->users[i].fd;
        }
    }

    return -1; 
}

int database_add_user(struct User* user) {
    struct Database* db = &database;

    if (!user) {
        fprintf(stderr, "Error: User pointer is NULL.\n");
        return -1;
    }

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
    db->users[db->num_users].status = USER_STATE_IDLE;
    db->users[db->num_users].card_id = -1;

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
    const struct Database* db = &database;
    return db->num_users;
}

int database_get_user_list(in_port_t* user_ports, int max_users) {
    const struct Database* db = &database;

    if (!user_ports || max_users <= 0) {
        return 0;
    }

    int count = (db->num_users < max_users) ? db->num_users : max_users;
    for (int i = 0; i < count; i++) {
        user_ports[i] = db->users[i].port;
    }
    return count;
}

static in_port_t last_chosen_port = 0;

void database_reset_user_iterator() {
    last_chosen_port = 0;
}

int database_get_next_user_port() {
    struct Database* db = &database;

    if (db->num_users == 0) {
        return -1;
    }

    // Find the minimum port among idle users
    int min_port_index = -1;
    for (int i = 0; i < db->num_users; i++) {
        if (db->users[i].status == USER_STATE_IDLE) {
            if (min_port_index == -1 || db->users[i].port < db->users[min_port_index].port) {
                min_port_index = i;
            }
        }
    }

    // No idle users found
    if (min_port_index == -1) {
        return -1;
    }

    if (last_chosen_port == 0) {
        last_chosen_port = db->users[min_port_index].port;
        return last_chosen_port;
    }

    in_port_t next_port = 0;
    int found = 0;
    for (int i = 0; i < db->num_users; i++) {
        if (db->users[i].status == USER_STATE_IDLE && db->users[i].port > last_chosen_port) {
            if (!found || db->users[i].port < next_port) {
                next_port = db->users[i].port;
                found = 1;
            }
        }
    }

    if (!found) {
        next_port = db->users[min_port_index].port;
    }

    last_chosen_port = next_port;

    return last_chosen_port;
}

void database_user_set_status(in_port_t user_id, enum UserStatus status) {
    struct Database* db = &database;

    int index = database_get_user_from_port(user_id);
    if (index == -1) {
        fprintf(stderr, "Error: User with ID %d not found.\n", user_id);
        return;
    }

    db->users[index].status = status;
}

void database_user_assign_card(in_port_t user_id, int card_id) {
    struct Database* db = &database;

    int index = database_get_user_from_port(user_id);
    if (index == -1) {
        fprintf(stderr, "Error: User with ID %d not found.\n", user_id);
        return;
    }

    db->users[index].card_id = card_id;
}

void database_print_users() {
    const struct Database* db = &database;

    fprintf(stdout, "==== Current Users in Database ====\n");
    if (db->num_users == 0) {
        fprintf(stdout, "> (no users)\n");
    } else {
        for (int i = 0; i < db->num_users; i++) {
            fprintf(stdout, "> User %d: Port=%d\n", i + 1, db->users[i].port);
        }
    }
    fprintf(stdout, "===================================\n");
}

int database_create_card(const char* text) {
    struct Database* db = &database;

    if (!text) {
        fprintf(stderr, "Error: Card text is NULL.\n");
        return -1;
    }

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

    append_card(&db->lavagna.todo, new_card);
    db->lavagna.num_cards++;

    return new_card->id;
}

int database_card_doing(in_port_t user_id) {
    struct Database* db = &database;

    int card_id = db->users[database_get_user_from_port(user_id)].card_id;
    if (card_id == -1) {
        fprintf(stderr, "Error: User %d has no assigned card.\n", user_id);
        return -1;
    }

    struct Card* card = remove_card_from_list(&db->lavagna.todo, card_id);

    card->status = CARD_STATUS_DOING;
    card->user = user_id;
    append_card(&db->lavagna.doing, card);

    return 0;
}

int database_card_done(int card_id) {
    struct Database* db = &database;

    struct Card* card = remove_card_from_list(&db->lavagna.doing, card_id);
    if (!card) {
        fprintf(stderr, "Error: Card %d not found in DOING list.\n", card_id);
        return -1;
    }

    card->status = CARD_STATUS_DONE;
    append_card(&db->lavagna.done, card);

    return 0;
}

int database_card_todo(in_port_t user_id) {
    struct Database* db = &database;

    struct User* user = &db->users[database_get_user_from_port(user_id)];
    int card_id = user->card_id;
    if (card_id == -1) {
        return -1;
    }
    user->card_id = -1;
    user->status = USER_STATE_IDLE;

    struct Card* card = remove_card_from_list(&db->lavagna.doing, card_id);

    card->status = CARD_STATUS_TODO;
    card->user = 0;
    append_card(&db->lavagna.todo, card);

    return 0;
}

int database_get_next_todo_card() {
    const struct Database* db = &database;

    if (db->lavagna.todo == NULL) {
        return -1;
    }

    return db->lavagna.todo->id;
}

int database_card_get_text(int card_id, char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return -1;
    }

    struct Card* card = database_find_card(card_id);
    if (!card) {
        buffer[0] = '\0';
        return -1;
    }

    strncpy(buffer, card->text, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';

    fprintf(stdout, "database_card_get_text: card_id=%d, text=%s\n", card_id, buffer);
    return 0;
}

void database_print_cards() {
    const struct Database* db = &database;

    fprintf(stdout, "==================== Current Cards in Lavagna ====================\n");

    const struct Card* current = db->lavagna.todo;
    fprintf(stdout, "-- TODO --\n");
    if (!current) {
        fprintf(stdout, "(empty)\n");
    }
    while (current) {
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->user, current->text);
        current = current->next;
    }

    current = db->lavagna.doing;
    fprintf(stdout, "-- DOING --\n");
    if (!current) {
        fprintf(stdout, "(empty)\n");
    }
    while (current) {
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->user, current->text);
        current = current->next;
    }

    current = db->lavagna.done;
    fprintf(stdout, "-- DONE --\n");
    if (!current) {
        fprintf(stdout, "(empty)\n");
    }
    while (current) {
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->user, current->text);
        current = current->next;
    }

    fprintf(stdout, "=================================================================\n");
}