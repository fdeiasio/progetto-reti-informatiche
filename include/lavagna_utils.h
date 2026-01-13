#ifndef LAVAGNA_UTILS_H
#define LAVAGNA_UTILS_H

#include "common.h"

#define WORKING_TIMEOUT_SECONDS 4
#define PING_TIMEOUT_SECONDS 2
#define TIMEOUT_CHECK_PERIOD 1

extern void send_user_list(int fd);

extern void assign_card();

extern int send_user_ping(int fd);

extern void check_timeout();

extern void send_user_quit(int fd);

extern void disconnect_user(in_port_t user_port);

#endif