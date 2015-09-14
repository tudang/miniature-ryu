#ifndef _NETPAXOS_MSG_H
#define _NETPAXOS_MSG_H

#include <time.h>
#define MAX_BUF 1440

typedef struct {
    char data[MAX_BUF];
    struct timespec time;
} netpaxos_message;

#endif
