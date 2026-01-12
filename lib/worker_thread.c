#include "pch.h"
#include "worker_thread.h"
#include "client.h"

#define MAX_SLEEP 10

void* worker_thread_function(void* arg) {
    struct Client* client = (struct Client*) arg;

    // this thread sleeps for a random amount of time between 1 and 5 seconds
    srand(time(NULL));
    
    int sleep_time = (rand() % MAX_SLEEP) + 1;
    fprintf(stdout, "Worker: starting work for %d seconds...\n", sleep_time);

    sleep(sleep_time);
    fprintf(stdout, "Worker: work completed.\n");

    client_update_state(client, STATE_WAITING_UL);
    pthread_exit(NULL);
}