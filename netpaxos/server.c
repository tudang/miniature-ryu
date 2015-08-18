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


pthread_t tid[4]; // this is thread identifier

struct timespec values[4][MAX_NUM];  // value queues
int start_receiving = 0;
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

/* This is thread function */
void *recvFunc(void *arg)
{
    interface *itf = (interface*) arg;
    int sock, n;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    int idx = itf->idx;

    sock=socket(AF_INET,SOCK_DGRAM,0);
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, 
                            itf->name, strlen(itf->name)) < 0) 
    {
        perror("Setsockopt Error\n");
        exit(1);    
    } 

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);
    bind(sock,(struct sockaddr *)&servaddr,sizeof(servaddr));
    
    struct timespec mesg = {0, 0};
    len = sizeof(cliaddr);
    int inst = 0;
    struct pollfd ufd = { .fd = sock, .events = POLLIN | POLLPRI, .revents = 0 };
    while(inst < MAX_NUM) {
        int rv = poll(&ufd, 1, 5000);
        if (rv == -1) {
            perror("poll");
            exit(-1);
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


void eval()
{
    int i,j;
    for (i = 0; i < MAX_NUM; i++) {
        for (j = 0; j < 4; j++) {
            struct timespec ts = values[j][i];
            printf("%lld.%.9ld\t", (long long)ts.tv_sec, ts.tv_nsec);
        }
        printf("\n");
    }
}


void usage(char* prog) {
    printf("Usage: %s eth*\n", prog);
     exit(1);
}
int main(int argc, char**argv)
{
    int i, err;
    int *ptr[4];
    if (argc < 2) usage(argv[0]);
    int cols = argc - 1;

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
    eval();

    /* release memory */ 
    for(i = 0; i < cols; i++) {
       free(nics[i]->name);
       free(nics[i]);
    }
    free(nics);
    return 0;
}
