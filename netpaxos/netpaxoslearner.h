#ifndef NETPAXOSLEARNER_H
#define NETPAXOSLEARNER_H
struct timespec values[4][MAX_NUM];  // value queues


typedef struct interface {
    //char *name;
    int idx;
    int instance;
} interface;


int run_learner(int argc, char **argv);

#endif
