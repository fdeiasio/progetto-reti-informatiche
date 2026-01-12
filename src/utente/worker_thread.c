#include "../../include/common.h"
#include "../../include/thread.h"
#include "../../include/utente_state.h"

#define MAX_SLEEP 10

/**
 * Il thread worker simula il lavoro sulla card
 * dormendo per un tempo casuale tra 1 e MAX_SLEEP secondi.
 */
void* worker_thread_function(void* arg) {
    (void) arg;
    
    srand(time(NULL));
    
    int sleep_time = (rand() % MAX_SLEEP) + 1;
    fprintf(stdout, "Worker: starting work for %d seconds...\n", sleep_time);

    sleep(sleep_time);

    // Notifico il completamento del lavoro
    fprintf(stdout, "Worker: work completed.\n");
    utente_update_state(STATE_WAITING_UL);

    pthread_exit(NULL);
}