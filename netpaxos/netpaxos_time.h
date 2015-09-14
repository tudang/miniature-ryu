#ifndef _NETPAXOS_TIME_H
#define _NETPAXOS_TIME_H

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

void gettime(struct timespec * ts);
uint64_t timediff(struct timespec start, struct timespec end);
int compare_ts(struct timespec time1, struct timespec time2);

#endif
