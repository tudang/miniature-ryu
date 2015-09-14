#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <event2/event.h>
#include <signal.h>

#include "config.h"
#include "netpaxos_learner.h"
#include "netpaxos_msg.h"
#include "netpaxos_time.h"
#include "netpaxos_util.h"

void signal_handler(evutil_socket_t fd, short what, void *arg) {
    struct event_base *base = (struct event_base*)arg;
    if (what&EV_SIGNAL) {
        event_base_loopbreak(base);
    }
}


/* This is thread function */
void recvFunc(evutil_socket_t sock, short what, void *arg) 
{
    interface *citf = (interface*)arg;
    int n;
    struct sockaddr_in cliaddr;
    socklen_t len;
    len = sizeof(cliaddr);
    if (citf->instance < citf->num_instance) {
        n = recvfrom(sock, &citf->values[citf->instance], sizeof(netpaxos_message), 
            0, (struct sockaddr*)&cliaddr, &len);
        if (n < 0) 
            perror("recvfrom");
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


void dump(int rows, struct interface *itf)
{
    int i;
    for (i = 0; i < rows; i++) {
            netpaxos_message *msg = &itf->values[i];
            fprintf(stderr, "%lld.%09ld\n", (long long)msg->time.tv_sec, msg->time.tv_nsec);
        }
}

int run_learner(int cols, int rows, char **argv) {
    struct event_base *base = event_base_new();
    if (!base) {
        puts("Couldn't get an event_base!");
    }
    struct event *recv;
    interface *itfx = malloc(sizeof(interface*));
    itfx->values = calloc(rows, sizeof(netpaxos_message));
    itfx->instance = 0;
    itfx->num_instance = rows;
    int sock = socket(AF_INET,SOCK_DGRAM,0);
    setRecBuf(sock);
    addMembership(sock, SERVER);
    bindSocket(sock, PORT);
    recv = event_new(base, sock, EV_READ|EV_PERSIST, recvFunc, itfx);
    event_add(recv, NULL);
    
    /* Signal event to terminate event loop */
    struct event *evsig;
    evsig = evsignal_new(base, SIGTERM, signal_handler, base);
    event_add(evsig, NULL);

    event_base_dispatch(base);
    dump(rows, itfx);

    /* release memory */ 
    event_base_free(base);
    free(itfx->values);
    free(itfx);

    return 0;
}

