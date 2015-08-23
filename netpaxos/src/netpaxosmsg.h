#ifndef _netpaxosmsg_h
#define _netpaxosmsg_h

#include <time.h>
#define MAX_BUF 1440

typedef struct {
    char data[MAX_BUF];
    struct timespec time;
} netpaxos_message;

#endif
