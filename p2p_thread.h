#ifndef P2P_THREAD_H
#define P2P_THREAD_H

#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "server.h"

void* p2p_server_function(void* arg);

#endif // P2P_THREAD_H