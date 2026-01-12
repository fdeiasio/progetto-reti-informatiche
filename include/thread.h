#ifndef P2P_THREAD_H
#define P2P_THREAD_H

#include "common.h"

/**
 * Questo header definisce le funzioni per i thread P2P e worker.
 * Viene incluso nei file sorgente che implementano questi thread.
 */

// Funzione del thread del server P2P
void* p2p_server_function(void* arg);

// Funzione del thread worker
void* worker_thread_function(void* arg);

#endif // P2P_THREAD_H