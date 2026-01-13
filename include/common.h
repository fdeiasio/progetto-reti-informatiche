#ifndef PCH_H
#define PCH_H

// Questo header contiene tutte le librerie usate nel progetto
// Sono raccolte qui per evitare di avere multipli include in ogni file
// Sono definite anche alcune costanti globali

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

#define LOCALHOST "127.0.0.1"
#define SERVER_PORT 5678

#endif //PCH_H