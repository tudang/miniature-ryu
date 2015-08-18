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

#include "config.h"


pthread_t tid[4]; // this is thread identifier

int values[4][MAX_NUM];  // value queues
int start_receiving = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int idx = 0;

pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;

int cont = 1; // share variable between main & eval thread

struct thread_args {
    int index;
    char *itf;
};

uint64_t timediff(struct timespec start, struct timespec end)
{
    return (BILLION * (end.tv_sec - start.tv_sec) +
                    end.tv_nsec - start.tv_nsec);
}

/* This is thread function */
void *recvFunc(void *arg)
{
    int sockfd, n;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    char mesg[BUF_SIZE];
    char *itf;
    itf = (char*)arg; 
    pthread_mutex_lock(&lock);
    int index = idx++;
    pthread_mutex_unlock(&lock);
    pthread_t self_id;
    self_id = pthread_self();
    char last_msg[9];
    int last_id = 0;
    int inst = 0;
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, 
                            itf, strlen(itf)) < 0) 
    {
        perror("Setsockopt Error\n");
        exit(1);    
    } 

 
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);
    bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    
    
    char buf[BUF_SIZE];
    len = sizeof(cliaddr);
    // Receive the first message
    n = recvfrom(sockfd,mesg,BUF_SIZE,0,(struct sockaddr *)&cliaddr,&len);
    strncpy(last_msg, mesg, 8);
    last_id = atoi(last_msg);
    values[index][inst++] = last_id;  
    n = sendto(sockfd,last_msg,strlen(last_msg), 0, (struct sockaddr *)&cliaddr, len);
    start_receiving = 1;

    // Subsequent messages
    while (inst < MAX_NUM)
    {
        n = recvfrom(sockfd,mesg,BUF_SIZE,0,(struct sockaddr *)&cliaddr,&len);
        strncpy(last_msg, mesg, 8);
        last_id = atoi(last_msg);
        values[index][inst] = last_id;  
        inst++;
        if (index == 0) n = sendto(sockfd,last_msg,strlen(last_msg), 0, (struct sockaddr *)&cliaddr, len);
    }
    pthread_exit(&last_id);
    return NULL;
}


void eval()
{
    int i,j;
    for (i = 0; i < MAX_NUM; i++) {
        for (j = 0; j < 4; j++) {
            printf("%-8d\t", values[j][i]);
        }
        printf("\n");
    }
}

int findMajorityElement(int* arr, int size) {
    int count = 0, i, majorityElement;
    for (i = 0; i < size; i++) {
        if (count == 0)
            majorityElement = arr[i];
        if (arr[i] == majorityElement)
            count++;
        else
            count--;
    }
    count = 0;
    for (i = 0; i < size; i++)
        if (arr[i] == majorityElement)
            count++;
    if (count > size/2)
        return majorityElement;
    return -1;
}



int main(int argc, char**argv)
{
    int i, err;
    int count = 0;
    int *ptr[4];
    if (argc != 5) { printf("Usage: ./server eth0.5 eth1.6 eth2.7 eth3.8\n"); exit(1);}

    for(i = 1; i < argc; i++) {
        err = pthread_create(&tid[count++], NULL, recvFunc, argv[i]);
        if (err != 0) {
            perror("Thread create Error");
            exit(1);
        }
    }

    for(i = 0; i < count; i++) {
       pthread_join(tid[i], (void**)&(ptr[i])); 
    }

    eval();
    return 0;
}
