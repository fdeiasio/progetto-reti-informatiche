#ifndef PCH_H
#define PCH_H

/**
 * Questo header include tutte le librerie standard usate nel progetto.
 * Viene incluso in tutti i file sorgente per evitare di dover includere
 * ripetutamente le stesse librerie.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define MIN_PORT    5679
#define SERVER_PORT 5678
#define LOCALHOST   "127.0.0.1"

#endif //PCH_H