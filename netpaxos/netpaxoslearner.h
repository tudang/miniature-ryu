#ifndef NETPAXOSLEARNER_H
#define NETPAXOSLEARNER_H

typedef struct interface {
    //char *name;
    int idx;
    int instance;
} interface;


int run_learner(int argc, char **argv);

#endif
