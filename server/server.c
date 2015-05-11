#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define PORT 8888
#define SIZE 1470

pthread_t tid[4]; // this is thread identifier

/* This is thread function */
void *recvFunc(void *arg)
{
    int sockfd,n;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    char mesg[SIZE];
    char *itf;
    itf = (char*)arg;
    char last_msg[9];
    int last_id = 0;
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
    
    for (;;)
    {
       len = sizeof(cliaddr);
       n = recvfrom(sockfd,mesg,SIZE,0,(struct sockaddr *)&cliaddr,&len);
       //sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
       printf("[%s]%.8s\n", itf, mesg);
        strncpy(last_msg, mesg, 8);
        last_msg[9] = '\0'; // place the null terminator
        last_id = atoi(last_msg);
        if (last_id >= 999900) break; 
    }
        pthread_exit(&last_id);
    return NULL;
}
int main(int argc, char**argv)
{
    int i, err;
    int count = 0;
    int *ptr[4];

    for(i = 1; i < argc; i++) {
        err = pthread_create(&tid[count++], NULL, recvFunc, argv[i]);
        if (err != 0) {
            perror("Thread create Error");
            exit(1);
        }
        else 
            printf("Thread created successfully\n");
    }

    for(i = 0; i < count; i++) {
       pthread_join(tid[i], (void**)&(ptr[i])); 
    }

    for(i = 0; i < count; i++) {
       printf("Last received msg: [%d]\n", *ptr[i]);
    }
}
