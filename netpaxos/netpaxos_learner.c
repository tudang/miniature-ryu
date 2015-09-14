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
#include "netpaxos_learner.h"
#include "netpaxos_msg.h"
#include "netpaxos_time.h"

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
#ifdef __MACH__ 

#else
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, 
                            iname, strlen(iname)) < 0) 
    {
        perror("Bind to device\n");
        exit(-1);
    }
#endif 
}

/* This is thread function */
void recvFunc(evutil_socket_t sock, short what, void *arg) 
{
    interface *citf = (interface*)arg;
    int n;
    struct sockaddr_in cliaddr;
    socklen_t len;

    netpaxos_message mesg;
    len = sizeof(cliaddr);
    if (citf->instance < citf->num_instance) {
        n = recvfrom(sock, &mesg, sizeof(mesg), 0, (struct sockaddr*)&cliaddr, &len);
        if (n < 0) 
            perror("recvfrom");
        citf->values[citf->instance] = mesg.time;  
        citf->instance++;
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


int findMajority(int instance, int size, interface **itf, struct timespec **majority)
{
    struct timespec *arr[size];
    int i;
    for (i = 0; i < size; i++) {
        arr[i] = &itf[i]->values[instance];
    }
    int count = 0;
    for (i = 0; i < size; i++) {
        if (count == 0) 
            *majority = arr[i];
        if (compare_ts(*arr[i], **majority) == 0)
            count++;
        else
            count--;
    }
    count = 0;
    for (i = 0; i < size; i++) {
        if (compare_ts(*arr[i], **majority) == 0)
            count++;
    }
    if (count > size/2)
        return 1;
    
    *majority = NULL;
    return 0;
}

void dump(int rows, int cols, struct interface **itf)
{
    int i,j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            struct timespec a = itf[j]->values[i];
            fprintf(stderr, "%lld.%.9ld\t", (long long)a.tv_sec, a.tv_nsec);
        }
        fprintf(stderr, "\n");
        struct timespec *ret = NULL;
        findMajority(i, cols, itf, &ret);
        if (ret)
            if (ret->tv_sec != 0)
                fprintf(stdout,"%d,%lld.%.9ld\n", i, (long long)ret->tv_sec, ret->tv_nsec);
            else
                fprintf(stdout,"%d,Indecision\n", i);
        else
            fprintf(stdout,"%d,Indecision\n", i);
    }
}

int run_learner(int cols, int rows, char **argv) {
    int i;
    struct event_base *base = event_base_new();
    if (!base) {
        puts("Couldn't get an event_base!");
    }
    struct event *packet_arrive[cols];
    interface **itf = malloc(cols * sizeof(interface*));
    for(i = 0; i < cols; i++) {
        interface *itfx = malloc(sizeof(interface*));
        itfx->values = calloc(rows, sizeof(struct timespec));
        itfx->idx = i;
        itfx->instance = 0;
        itfx->num_instance = rows;
        int sock = socket(AF_INET,SOCK_DGRAM,0);
        setRecBuf(sock);
        bindToDevice(sock, argv[i]);
        bindSocket(sock, PORT);
        packet_arrive[i] = event_new(base, sock, EV_READ|EV_PERSIST, recvFunc, itfx);
        event_add(packet_arrive[i], NULL);
        itf[i] = itfx;
    }
    
    /* Signal event to terminate event loop */
    struct event *evsig;
    evsig = evsignal_new(base, SIGTERM, signal_handler, base);
    event_add(evsig, NULL);

    event_base_dispatch(base);
    dump(rows, cols, itf);

    /* release memory */ 
    event_base_free(base);
    for(i = 0; i < cols; i++) {
        free(itf[i]->values);
        free(itf[i]);
    }
    free(itf);
    return 0;
}

void setRecBuf(int sock) {
    int rcvbuf = 16777216;
    int rc = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(sock));
    if (rc == -1) {
        perror("setRecBuf");
        exit(-1);
    }
}


int getRecBuf(int sock) {
    int rcvbuf = 0;
    socklen_t size = sizeof(sock);
    int rc = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &size);
    if (rc == -1) {
        perror("setRecBuf");
        exit(-1);
    }
    return rcvbuf;
}