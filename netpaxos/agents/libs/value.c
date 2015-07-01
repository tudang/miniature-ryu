#include "value.h"

uint64_t timediff(struct timespec start, struct timespec end)
{
    return (BILLION * (end.tv_sec - start.tv_sec) +
                    end.tv_nsec - start.tv_nsec);
}


paxosval new_value(int inst, int crnd, int vrnd, int vval) {
    paxosval p;
    p.instance = inst;
    p.crnd = crnd;
    p.vrnd = vrnd;
    p.vval = vval;
    return p;
}

void netpaxos_to_string(char *str, paxosval p) {
    sprintf(str, "%d,%d,%d,%d\n", p.instance, p.crnd,
                p.vrnd, p.vval);
}


void header_to_string(char *str, struct header hd) {
    sprintf(str, "%d,%d,%d,%d.%d,%d", hd.msg_type, hd.client_id,
                hd.sequence, hd.ts.tv_sec, hd.ts.tv_nsec, hd.buffer_size);
}
