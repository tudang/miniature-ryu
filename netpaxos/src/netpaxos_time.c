#include "netpaxos_time.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "stdint.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>

#endif

void gettime(struct timespec * ts) {      
#ifdef __MACH__ 
        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        ts->tv_sec = mts.tv_sec;
        ts->tv_nsec = mts.tv_nsec;

#else
        clock_gettime(CLOCK_REALTIME, ts);
#endif
}

uint64_t timediff(struct timespec start, struct timespec end)
{
    return (1e9 * (end.tv_sec - start.tv_sec) +
                    end.tv_nsec - start.tv_nsec);
}

