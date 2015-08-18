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

#include "config.h"



struct timespec values[4][MAX_NUM];  // value queues
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


typedef struct interface {
    char *name;
    int idx;
} interface;

uint64_t timediff(struct timespec start, struct timespec end)
{
    return (BILLION * (end.tv_sec - start.tv_sec) +
                    end.tv_nsec - start.tv_nsec);
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
void *recvFunc(void *arg)
{
    interface *itf = (interface*) arg;
    int sock, n;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    int idx = itf->idx;

    sock=socket(AF_INET,SOCK_DGRAM,0);
    bindToDevice(sock, itf->name);
    bindSocket(sock, PORT);
    
    struct timespec mesg = {0, 0};
    len = sizeof(cliaddr);
    int inst = 0;
    struct pollfd ufd = { .fd = sock, .events = POLLIN | POLLPRI, .revents = 0 };
    while(inst < MAX_NUM) {
        int rv = poll(&ufd, 1, 5000);
        if (rv == -1) {
            perror("poll");
            exit(-2);
        }
        else if (rv == 0) {
            break;
        } 
        else {
            n = recvfrom(sock,&mesg,sizeof(mesg),0,(struct sockaddr *)&cliaddr,&len);
            if (n < 0) 
                perror("recvfrom");
            values[idx][inst] = mesg;  
            inst++;
            if (idx == 0) {
                n = sendto(sock,&mesg,sizeof(mesg), 0, (struct sockaddr *)&cliaddr, len);
                if (n < 0) 
                    perror("sendto");
            }
       }
    }
    return NULL;
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


void usage(char* prog) {
    fprintf(stderr, "Usage: %s eth*\n", prog);
     exit(1);
}
int main(int argc, char**argv)
{
    int i, err;
    if (argc < 2) usage(argv[0]);
    int cols = argc - 1;
    pthread_t tid[cols];

    interface **nics = calloc(cols, sizeof(interface));
    for(i = 0; i < cols; i++) {
        interface *nic_x = malloc(sizeof(interface));
        nic_x->idx = i;
        nic_x->name = strdup(argv[i+1]);
        err = pthread_create(&tid[i], NULL, recvFunc, nic_x);
        if (err != 0) {
            perror("Thread create Error");
            exit(1);
        }
        nics[i] = nic_x;
    }

    for(i = 0; i < cols; i++) {
       pthread_join(tid[i], NULL); 
    }

    dump();

    /* release memory */ 
    for(i = 0; i < cols; i++) {
       free(nics[i]->name);
       free(nics[i]);
    }
    free(nics);
    return 0;
}
