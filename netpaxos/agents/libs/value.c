#include "value.h"

void serialize_value(value v, char* buffer) {
    int MSG_SIZE = 28;
    char msgid[MSG_SIZE];
    sprintf(msgid, "%2d%06d%lld.%.9ld", v.client_id, v.sequence,
            (long long) v.ts.tv_sec, v.ts.tv_nsec);
            strncpy(buffer, msgid, MSG_SIZE);
}


void deserialize_value(char *buffer, value *v) {
    char cid[2];
    char seq[6];
    char ts[20];
    strncpy(cid, buffer, 2);
    v->client_id = atoi(cid);
    strncpy(seq, buffer+2, 6);
    v->sequence = atoi(seq);
    //printf("client_id:%d, seq:%d\n", v->client_id, v->sequence);
}


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
