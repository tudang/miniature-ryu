/* UDP client in the internet domain */
#ifndef _netpaxosclient_h_
#define _netpaxosclient_h_

#include <netinet/in.h>
#include <arpa/inet.h>

struct client {
    struct sockaddr_in dest;
    int count;
    int num_packet;
    int total;
};

int run_client(int interval, int num_packet);
#endif
