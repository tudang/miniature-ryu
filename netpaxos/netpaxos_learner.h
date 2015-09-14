#ifndef _NETPAXOS_LEARNER_H
#define _NETPAXOS_LEARNER_H
#include "netpaxos_msg.h"


typedef struct interface {
    int instance;
    int num_instance;
    netpaxos_message *values;
} interface;


int run_learner(int cols, int rows, char **argv);
#endif
