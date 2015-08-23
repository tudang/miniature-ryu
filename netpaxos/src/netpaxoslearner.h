#ifndef _netpaxoslearner_h_
#define _netpaxoslearner_h_
#include <time.h>

typedef struct interface {
    //char *name;
    int idx;
    int instance;
    int num_instance;
    struct timespec *values;
} interface;


int run_learner(int cols, int rows, char **argv);
void setRecBuf(int sock);
int getRecBuf(int sock);
#endif
