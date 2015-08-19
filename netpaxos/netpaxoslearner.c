#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/poll.h>
#include <event2/event.h>
#include <signal.h>

#include "config.h"
#include "netpaxoslearner.h"



uint64_t timediff(struct timespec start, struct timespec end)
{
    return (BILLION * (end.tv_sec - start.tv_sec) +
                    end.tv_nsec - start.tv_nsec);
}


void signal_handler(evutil_socket_t fd, short what, void *arg) {
    struct event_base *base = (struct event_base*)arg;
    if (what&EV_SIGNAL) {
        event_base_loopbreak(base);
    }
}

void bindSocket(int sock, int port) {
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(port);
    bind(sock,(struct sockaddr *)&servaddr,sizeof(servaddr));

}


void bindToDevice(int sock, char *iname) {
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, 
                            iname, strlen(iname)) < 0) 
    {
        perror("Bind to device\n");
        exit(-1);
    } 
}

/* This is thread function */
void recvFunc(evutil_socket_t sock, short what, void *arg) 
//void *recvFunc(void *arg)
{
    interface *itf = (interface*)arg;
    int idx = itf->idx;
    int n;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;

    struct timespec mesg = {0, 0};
    len = sizeof(cliaddr);
    if (itf->instance < MAX_NUM) {
        n = recvfrom(sock,&mesg,sizeof(mesg),0,(struct sockaddr *)&cliaddr,&len);
        if (n < 0) 
            perror("recvfrom");
        values[idx][itf->instance] = mesg;  
        itf->instance++;
        /* respond to client 
        if (idx == 0) {
            n = sendto(sock,&mesg,sizeof(mesg), 0, (struct sockaddr *)&cliaddr, len);
            if (n < 0) 
                perror("sendto");
        }
        */
    }
}


int compare_ts(struct timespec time1, struct timespec time2) {
        if (time1.tv_sec < time2.tv_sec)
            return (-1) ;               /* Less than. */
        else if (time1.tv_sec > time2.tv_sec)
            return (1) ;                /* Greater than. */
        else if (time1.tv_nsec < time2.tv_nsec)
            return (-1) ;               /* Less than. */
        else if (time1.tv_nsec > time2.tv_nsec)
            return (1) ;                /* Greater than. */
        else
            return (0) ;                /* Equal. */
}


struct timespec findMajority(int i)
{
    struct timespec ret  = {0,0};
    struct timespec a = values[0][i];
    struct timespec b = values[1][i];
    struct timespec c = values[2][i];
    struct timespec d = values[3][i];
    if (compare_ts(a,b) == 0) {
        if (compare_ts(a,c) == 0 || compare_ts(a,d) == 0) 
            return a;
    }
    else { 
        if (compare_ts(a,c) == 0 && compare_ts(a,d) == 0) 
            return a;
        else if (compare_ts(b,c) == 0 && compare_ts(b,d) == 0) 
            return b;
        else
            return ret;
    }
}

void dump()
{
    int i,j;
    for (i = 0; i < MAX_NUM; i++) {
        for (j = 0; j < 4; j++) {
            struct timespec a = values[j][i];
            fprintf(stderr, "%lld.%.9ld\t", (long long)a.tv_sec, a.tv_nsec);
        }
        fprintf(stderr, "\n");
        struct timespec ret = findMajority(i);
        if (ret.tv_sec != 0 && ret.tv_nsec != 0)
            fprintf(stdout,"%lld.%.9ld\n", (long long)ret.tv_sec, ret.tv_nsec);
        else
            fprintf(stdout,"Indecision\n");
    }
}




int run_learner(int cols, char **argv) {
    int i, err;
    struct event_base *base = event_base_new();
    if (!base) {
    puts("Couldn't get an event_base!");
    }
    struct event *packet_arrive[cols];

    for(i = 0; i < cols; i++) {
        interface *itf = malloc(sizeof(interface));
        itf->idx =  i;
        itf->instance = 0;
        int sock = socket(AF_INET,SOCK_DGRAM,0);
        bindToDevice(sock, argv[i]);
        bindSocket(sock, PORT);
        packet_arrive[i] = event_new(base, sock, EV_READ|EV_PERSIST, recvFunc, itf);
        event_add(packet_arrive[i], NULL);
    }
    
    /* Signal event to terminate event loop */
    struct event *evsig;
    evsig = evsignal_new(base, SIGTERM, signal_handler, base);
    event_add(evsig, NULL);

    event_base_dispatch(base);
    dump();

    /* release memory */ 
    event_base_free(base);
    return 0;
}
