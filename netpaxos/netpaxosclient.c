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
#include <event2/event.h>
#include <signal.h>

#include "config.h"
#include "netpaxosclient.h"
#include "netpaxosmsg.h"

int count; 

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

void sig_handler(evutil_socket_t fd, short what, void *arg) {
    struct event_base *base = (struct event_base*)arg;
    if (what&EV_SIGNAL) {
        event_base_loopbreak(base);
    }
}

void read_cb(evutil_socket_t sock, short what, void *arg) 
{
    if (what&EV_READ) {
        netpaxos_message msg;
        struct timespec end;
        int n = recvfrom(sock, &msg, sizeof(msg), 0, NULL, NULL);
        if (n < 0) error("recvfrom");
        clock_gettime(CLOCK_REALTIME, &end);
        uint64_t diff = tsdiff(msg.time, end);
        printf("%ld\n", diff / 2000);
    }
}

void send_cb(evutil_socket_t sock, short what, void *arg) 
{
    if (what&EV_TIMEOUT) {
        struct client *c = (struct client*)arg;
        struct sockaddr_in server_addr = c->dest;
        unsigned int length;
        length = sizeof(struct sockaddr_in);
        struct timespec tsp;
        if (c->count < c->num_packet) {
            // get timestamp and send
            clock_gettime(CLOCK_REALTIME, &tsp);
            netpaxos_message msg;
            msg.time = tsp;
            int n = sendto(sock, &msg, sizeof(msg), 0, 
                       (struct sockaddr *)&server_addr, length);
            if (n < 0) error("sendto");
            printf("send %d bytes\n", n);
            c->total += n;
            c->count++;
        }
    }
}


int run_client(int interval, int num_packet) 
{
    count = 0;
    struct sockaddr_in server_addr;
    struct hostent *hp;
    struct event_base *base = event_base_new();
    if (!base) {
        puts("Couldn't get an event_base!");
    }
    
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

    struct client c = { .dest = server_addr, .count = 0, .num_packet = num_packet, .total = 0.0 };

    struct event *evread = event_new(base, sock, EV_READ|EV_PERSIST, read_cb, NULL);
    event_add(evread, NULL);
    struct timeval rest = {0,interval};
    struct event *evsend = event_new(base, sock, EV_TIMEOUT|EV_PERSIST, send_cb, &c);
    event_add(evsend, &rest);


    struct timespec tstart, tend;

    /* Signal event to terminate event loop */
    struct event *evsig;
    evsig = evsignal_new(base, SIGTERM, sig_handler, base);
    event_add(evsig, NULL);

    // get time start sending
    clock_gettime(CLOCK_REALTIME, &tstart);
    event_base_dispatch(base);

    // get time end sending
    clock_gettime(CLOCK_REALTIME, &tend);
    float duration = tsdiff(tstart, tend) / BILLION;

    printf("packets/second: %3.2f\n", (float) c.count / duration);
    /* wait for our thread to finish before continuing */
    return 0;
}
