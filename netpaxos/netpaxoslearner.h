#ifndef NETPAXOSLEARNER_H
#define NETPAXOSLEARNER_H
#include <time.h>

typedef struct interface {
    //char *name;
    int idx;
    int instance;
    int num_instance;
    struct timespec *values;
} interface;


int run_learner(int cols, int rows, char **argv);

#endif
