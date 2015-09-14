#ifndef _netpaxos_time_h_
#define _netpaxos_time_h_

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

void gettime(struct timespec * ts);
uint64_t timediff(struct timespec start, struct timespec end);
#endif
