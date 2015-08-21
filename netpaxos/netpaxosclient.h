/* UDP client in the internet domain */
#include <netinet/in.h>
#include <arpa/inet.h>

struct client {
    struct sockaddr_in dest;
    int count;
    int num_packet;
    int total;
};

int run_client(int interval, int num_packet);
