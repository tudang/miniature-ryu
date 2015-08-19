/* UDP client in the internet domain */
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <ctype.h>
#include <sys/poll.h>

#include "config.h"
#include "netpaxosclient.h"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

uint64_t tsdiff(struct timespec start, struct timespec end)
{
    return (BILLION * (end.tv_sec - start.tv_sec) +
                    end.tv_nsec - start.tv_nsec);
}


void *recvMsg(void *arg)
{
    struct server *s = (struct server*) arg;
    int sock = s->socket;
    int count = 0;
    struct timespec ret;
    double total_latency = 0.0;
    struct pollfd ufd = { .fd = sock, .events = POLLIN | POLLPRI, .revents = 0 };
    while(1) {
        int rv = poll(&ufd, 1, 1000);
        if (rv == -1) {
            perror("poll");
            exit(-1);
        }
        else if (rv == 0) {
            return NULL;
        } 
        else {
            int n = recvfrom(sock, &ret, sizeof(ret), 0, NULL, NULL);
            if (n < 0) error("recvfrom");
            struct timespec end;
            clock_gettime(CLOCK_REALTIME, &end);
            uint64_t diff = tsdiff(ret, end);
            total_latency += (diff / 2000);
            count++;
        }
    }
    printf("average latency: %3.2f\n", (total_latency / count));
    return NULL;
}

int run_client(int interval, int num_packets) 
{
    pthread_t sth, rth; // thread identifier
    struct sockaddr_in server_addr;
    struct hostent *hp;
    unsigned int length;
    struct server *serv;
    int c;

    serv = malloc(sizeof(struct server));

    /* Create socket */
    int sock;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("socket");
    
    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fcntl error");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;
    hp = gethostbyname(SERVER);
    if (hp == NULL) error("Uknown host");

    bcopy((char *)hp->h_addr,
         (char *)&server_addr.sin_addr,
         hp->h_length);
    server_addr.sin_port = htons(PORT);
    length = sizeof(struct sockaddr_in);

    (*serv).socket = sock;
    (*serv).server = server_addr;
    (*serv).length = length;


    /* Create worker thread */
    pthread_create(&rth, NULL, recvMsg, (void*) serv);

    /* Sending in Main thread */

    int total = 0, count = 0;
    struct timespec tsp, tstart, tend;
    struct timespec req = {0, interval};


    // get time start sending
    clock_gettime(CLOCK_REALTIME, &tstart);

    struct pollfd ufds = { .fd = sock, .events = POLLOUT, .revents = 0 };

    while (count < num_packets) {
                int rv = poll(&ufds, 1, 1000);
        if (rv == -1) {
            perror("poll");
        } else if (rv == 0) {
            perror("timeout occured");
        }
        else {
                // get timestamp and send
                clock_gettime(CLOCK_REALTIME, &tsp);
                int n = sendto(sock, &tsp, sizeof(tsp), 0, 
                            (struct sockaddr *)&server_addr, length);
                if (n < 0) error("sendto");
                total += n;
            count++;
            nanosleep(&req, NULL);
        }
    }
    // get time end sending
    clock_gettime(CLOCK_REALTIME, &tend);
    float duration = tsdiff(tstart, tend) / BILLION;

    printf("packets/second: %3.2f\n", (float) count / duration);
    /* wait for our thread to finish before continuing */
    pthread_cancel(rth);
    pthread_join(rth, NULL);
    free(serv);
    return 0;
}
