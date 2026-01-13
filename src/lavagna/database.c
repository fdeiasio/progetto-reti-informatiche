#include "../../include/common.h"
#include "../../include/database.h"

// Struttura globale del database
struct Database database;

// Funzioni statiche di supporto

static const char* STATUS_TO_STRING[] = {
    [USER_STATUS_ACTIVE] = "ACTIVE",
    [USER_STATUS_IDLE] = "IDLE",
    [USER_STATUS_PINGED] = "PINGED",
    [USER_STATUS_ERROR] = "ERROR",
};

// Libera una lista di card
static void free_card_list(struct Card* head) {
    while (head) {
        struct Card* next = head->next;
        free(head);
        head = next;
    }
}

// Libera una lista di utenti
static void free_user_list(struct User* head) {
    while (head) {
        struct User* next = head->next;
        free(head);
        head = next;
    }
}

// Aggiunge una card in coda alla lista
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

// Aggiunge un utente in ordine di porta
static void insert_user(struct User** head, struct User* user) {
    if (*head == NULL || (*head)->port > user->port) {
        user->next = *head;
        *head = user;
    } else {
        struct User* current = *head;
        while (current->next != NULL && current->next->port < user->port) {
            current = current->next;
        }
        user->next = current->next;
        current->next = user;
    }
}

// Rimuove una card dalla lista in base all'id
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

// Rimuove un utente dalla lista in base alla porta
static struct User* remove_user_from_list(struct User** head, in_port_t user_id) {
    for (struct User** curr = head; *curr != NULL; curr = &(*curr)->next) {
        if ((*curr)->port == user_id) {
            struct User* user = *curr;
            *curr = user->next;
            user->next = NULL;
            return user;
        }
    }
    return NULL;
}

// Trova una card nella lista in base all'id
static struct Card* find_card_in_list(struct Card* head, int card_id) {
    for (struct Card* curr = head; curr != NULL; curr = curr->next) {
        if (curr->id == card_id) {
            return curr;
        }
    }
    return NULL;
}

// Trova un utente nella lista in base alla porta
static struct User* find_user_in_list(struct User* head, in_port_t user_id) {
    for (struct User* curr = head; curr != NULL; curr = curr->next) {
        if (curr->port == user_id) {
            return curr;
        }
    }
    return NULL;
}

// Trova una card nel database in base all'id
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

// Funzioni pubbliche

void database_init() {
    struct Database* db = &database;

    db->users = NULL;
    db->num_users = 0;

    db->lavagna = (struct Lavagna) {
        .id = 1,
        .num_cards = 0,
        .todo = NULL,
        .doing = NULL,
        .done = NULL,
    };
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

    free_user_list(db->users);

    db->users = NULL;
    db->num_users = 0;
}

int database_add_user(int socket, in_port_t port) {
    struct Database* db = &database;

    if (find_user_in_list(db->users, port) != NULL) {
        return -1;
    }

    struct User* new_user = (struct User*)malloc(sizeof(struct User));
    if (!new_user) {
        return -1;
    }

    new_user->socket = socket;
    new_user->port = port;
    new_user->status = USER_STATUS_IDLE;
    new_user->assigned_card_id = -1;
    new_user->last_ping_timestamp = 0;
    new_user->next = NULL;

    insert_user(&db->users, new_user);
    db->num_users++;

    return 0;
}

int database_remove_user(in_port_t user_id) {
    struct Database* db = &database;

    struct User* removed_user = remove_user_from_list(&db->users, user_id);
    if (!removed_user) {
        return -1;
    }

    free(removed_user);
    db->num_users--;

    return 0;
}

int database_get_num_users() {
    const struct Database* db = &database;

    return db->num_users;
}

int database_get_users_list(in_port_t** user_ports) {
    const struct Database* db = &database;

    int count = db->num_users;
    if (count == 0) {
        *user_ports = NULL;
        return 0;
    }

    *user_ports = (in_port_t*)malloc(count * sizeof(in_port_t));
    if (!*user_ports) {
        return 0;
    }
    struct User* current = db->users;
    for (int i = 0; i < count; i++) {
        (*user_ports)[i] = current->port;
        current = current->next;
    }

    return count;
}

in_port_t database_get_port_from_socket(int socket) {
    struct Database* db = &database;

    for (struct User* curr = db->users; curr != NULL; curr = curr->next) {
        if (curr->socket == socket) {
            return curr->port;
        }
    }

    return 0; 
}

int database_get_socket_from_port(in_port_t port) {
    struct Database* db = &database;

    struct User* user = find_user_in_list(db->users, port);
    if (!user) {
        return -1;
    }

    return user->socket;
}

int database_user_set_status(in_port_t user_id, enum UserStatus status) {
    struct Database* db = &database;

    struct User* user = find_user_in_list(db->users, user_id);
    if (!user) {
        return -1;
    }

    user->status = status;

    return 0;
}

enum UserStatus database_user_get_status(in_port_t user_id) {
    struct Database* db = &database;

    struct User* user = find_user_in_list(db->users, user_id);
    if (!user) {
        return USER_STATUS_ERROR;
    }

    return user->status;
}


int database_user_assign_card(in_port_t user_id, int card_id) {
    struct Database* db = &database;

    struct User* user = find_user_in_list(db->users, user_id);
    if (!user) {
        return -1;
    }

    user->assigned_card_id = card_id;
    user->status = USER_STATUS_ACTIVE;

    return 0;
}

int database_user_set_ping(in_port_t user_id) {
    struct Database* db = &database;

    struct User* user = find_user_in_list(db->users, user_id);
    if (!user) {
        return -1;
    }

    user->status = USER_STATUS_PINGED;
    user->last_ping_timestamp = time(NULL);

    return 0;
}

int database_user_clear_ping(in_port_t user_id) {
    struct Database* db = &database;

    struct User* user = find_user_in_list(db->users, user_id);
    if (!user) {
        return -1;
    }

    user->status = USER_STATUS_ACTIVE;
    user->last_ping_timestamp = 0;

    return 0;
}

static struct User* last_chosen_user = NULL;

int database_get_next_user_port() {
    struct Database* db = &database;

    if (db->num_users == 0) {
        return -1;
    }

    struct User* start = last_chosen_user ? last_chosen_user->next : db->users;
    if (start == NULL) {
        start = db->users;
    }
    
    for (struct User* curr = start; curr != NULL; curr = curr->next) {
        if (curr->status == USER_STATUS_IDLE) {
            last_chosen_user = curr;
            return curr->port;
        }
    }

    for (struct User* curr = db->users; curr != start; curr = curr->next) {
        if (curr->status == USER_STATUS_IDLE) {
            last_chosen_user = curr;
            return curr->port;
        }
    }

    return -1;
}

// Puntatore statico all'utente in stato pending
static struct User* pending_user = NULL;

int database_user_set_pending(int socket) {
    struct Database* db = &database;

    for (struct User* curr = db->users; curr != NULL; curr = curr->next) {
        if (curr->socket == socket) {
            pending_user = curr;

            return 0;
        }
    }

    return -1;
}

int database_get_pending_user_socket() {
    if (pending_user) {
        return pending_user->socket;
    }

    return -1;
}

void database_user_clear_pending() {
    if (pending_user) {
        pending_user = NULL;
    }
}

int database_user_get_timed_out(in_port_t** user_ports, int timeout) {
    struct Database* db = &database;
    int count = 0;
    time_t now = time(NULL);

    *user_ports = (in_port_t*)malloc(db->num_users * sizeof(in_port_t));
    if (!*user_ports) {
        return 0;
    }

    for (struct User* curr = db->users; curr != NULL; curr = curr->next) {
        if (curr->status == USER_STATUS_PINGED) {
            double diff = difftime(now, curr->last_ping_timestamp);
            if (diff >= timeout) {
                (*user_ports)[count++] = curr->port;
            }
        }
    }

    return count;
}

void database_print_users() {
    const struct Database* db = &database;

    fprintf(stdout, "==== Current Users in Database ====\n");
    if (db->num_users == 0) {
        fprintf(stdout, "(no users)\n");
    } else {
        for (struct User* curr = db->users; curr != NULL; curr = curr->next) {
            fprintf(stdout, "User Port: %d\t Socket: %d\t Status: %s\t Card ID: %d\n", 
                curr->port, 
                curr->socket, 
                STATUS_TO_STRING[curr->status], 
                curr->assigned_card_id
            );
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
    new_card->assigned_user_port = 0;
    new_card->last_activity_timestamp = 0;

    append_card(&db->lavagna.todo, new_card);
    db->lavagna.num_cards++;

    return new_card->id;
}

ssize_t database_card_get_text(int card_id, char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return -1;
    }

    struct Card* card = database_find_card(card_id);
    if (!card) {
        buffer[0] = '\0';
        return -1;
    }

    size_t len = strlen(card->text);
    if (len >= buffer_size) {
        buffer[0] = '\0';
        return -1;
    }

    memcpy(buffer, card->text, len + 1);

    return len;
}

int database_card_doing(in_port_t user_id) {
    struct Database* db = &database;

    struct User* user = find_user_in_list(db->users, user_id);
    if (!user) {
        return -1;
    }

    int card_id = user->assigned_card_id;
    if (card_id == -1) {
        return -1;
    }

    struct Card* card = remove_card_from_list(&db->lavagna.todo, card_id);

    card->status = CARD_STATUS_DOING;
    card->assigned_user_port = user_id;
    card->last_activity_timestamp = time(NULL);
    append_card(&db->lavagna.doing, card);

    return 0;
}

int database_card_done(in_port_t user_id) {
    struct Database* db = &database;

    struct User* user = find_user_in_list(db->users, user_id);
    if (!user) {
        return -1;
    }

    int card_id = user->assigned_card_id;
    if (card_id == -1) {
        return -1;
    }

    struct Card* card = remove_card_from_list(&db->lavagna.doing, card_id);
    if (!card) {
        return -1;
    }

    user->status = USER_STATUS_IDLE;
    user->assigned_card_id = -1;

    card->status = CARD_STATUS_DONE;
    card->last_activity_timestamp = time(NULL);
    append_card(&db->lavagna.done, card);

    return 0;
}

int database_card_todo(in_port_t user_id) {
    struct Database* db = &database;

   struct User* user = find_user_in_list(db->users, user_id);
    if (!user) {
        return -1;
    }

    int card_id = user->assigned_card_id;
    if (card_id == -1) {
        return -1;
    }

    user->assigned_card_id = -1;
    user->status = USER_STATUS_IDLE;

    struct Card* card = remove_card_from_list(&db->lavagna.doing, card_id);

    card->status = CARD_STATUS_TODO;
    card->assigned_user_port = 0;
    card->last_activity_timestamp = 0;
    append_card(&db->lavagna.todo, card);

    return 0;
}

in_port_t database_card_get_user(int card_id) {
    struct Card* card = database_find_card(card_id);
    if (!card) {
        return -1;
    }

    return card->assigned_user_port;
}


int database_card_reset_timestamp(in_port_t user_id) {
    struct Database* db = &database;

    struct User* user = find_user_in_list(db->users, user_id);
    if (!user) {
        return -1;
    }

    int card_id = user->assigned_card_id;
    if (card_id == -1) {
        return -1;
    }

    struct Card* card = find_card_in_list(db->lavagna.doing, card_id);
    if (!card) {
        return -1;
    }

    card->last_activity_timestamp = time(NULL);

    return 0;
}

int database_get_next_todo_card() {
    const struct Database* db = &database;

    if (db->lavagna.todo == NULL) {
        return -1;
    }

    return db->lavagna.todo->id;
}

int database_card_get_timed_out(int** card_ids, int timeout) {
    struct Database* db = &database;
    int count = 0;
    time_t now = time(NULL);

    *card_ids = (int*)malloc(db->lavagna.num_cards * sizeof(int));
    if (!*card_ids) {
        return 0;
    }

    for (struct Card* curr = db->lavagna.doing; curr != NULL; curr = curr->next) {
        double diff = difftime(now, curr->last_activity_timestamp);
        if (diff >= timeout) {
            (*card_ids)[count++] = curr->id;
        }
    }

    return count;
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
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->assigned_user_port, current->text);
        current = current->next;
    }

    current = db->lavagna.doing;
    fprintf(stdout, "-- DOING --\n");
    if (!current) {
        fprintf(stdout, "(empty)\n");
    }
    while (current) {
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->assigned_user_port, current->text);
        current = current->next;
    }

    current = db->lavagna.done;
    fprintf(stdout, "-- DONE --\n");
    if (!current) {
        fprintf(stdout, "(empty)\n");
    }
    while (current) {
        fprintf(stdout, "Card ID: %d\t User: %d\t Text: %s\n", current->id, current->assigned_user_port, current->text);
        current = current->next;
    }

    fprintf(stdout, "=================================================================\n");
}