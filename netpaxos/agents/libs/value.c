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

void error(const char *msg) {
    perror(msg);
    exit(1);
}

char *get_interface_addr(char *itfname) {
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, itfname, IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    /* display result */
    char *itf_addr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    return itf_addr;
}

