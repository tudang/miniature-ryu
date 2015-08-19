/* UDP client in the internet domain */
#include <netinet/in.h>
#include <arpa/inet.h>

struct server {
    int socket;
    struct sockaddr_in server;
    unsigned int length;
};

int run_client(int interval, int num_packet);
