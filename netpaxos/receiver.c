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
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#define GROUP "239.0.0.1"
#define PORT 8888
#define SIZE 1470
#define MAX_NUM 500000
#define BILLION 1000000000L


uint64_t timediff(struct timespec start, struct timespec end)
{
    return (BILLION * (end.tv_sec - start.tv_sec) +
                    end.tv_nsec - start.tv_nsec);
}


/* This is thread function */
void *recvFunc(void *arg)
{
    int sockfd, n;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char mesg[SIZE];
    char *itf;
    itf = (char*)arg; 
    pthread_t self_id;
    self_id = pthread_self();
    char last_msg[9];
    int last_id = 0;
    int inst = 0;

    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if (sockfd < 0) {
        error("ERROR opening socket");
        exit(1);
    }

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


    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, itf, IFNAMSIZ-1);
    ioctl(sockfd, SIOCGIFADDR, &ifr);
    struct in_addr sin = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
    char itf_addr[INET_ADDRSTRLEN]; 
    if (inet_ntop(AF_INET, &sin, itf_addr, sizeof(itf_addr)) == NULL)
        perror("inet_ntop");
    
    struct ip_mreq mreq;

    mreq.imr_multiaddr.s_addr = inet_addr(GROUP);
    mreq.imr_interface.s_addr = inet_addr(itf_addr);

    if (setsockopt(sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))<0) {
        error("setsockopt mreq");
        exit(1);
    }
    
    if (bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        error("bind");
        exit(1);
    }

    FILE *out;
    char filename[20];
    sprintf(filename, "/tmp/%s.capture", itf);
    out = fopen(filename, "w");

    if (out == NULL) {
        perror("Error opening file");
        exit(1);
    }

    char buf[SIZE];
    len = sizeof(cliaddr);
    // Receive the first message
    n = recvfrom(sockfd,mesg,SIZE,0,(struct sockaddr *)&cliaddr,&len);
    strncpy(last_msg, mesg, 8);
    last_id = atoi(last_msg);
    fprintf(out, "%d\n", last_id);
    n = sendto(sockfd,last_msg,strlen(last_msg),0,(struct sockaddr *)&cliaddr,len);

    // Subsequent messages
    while (inst < MAX_NUM)
    {
        n=recvfrom(sockfd,mesg,SIZE,0,(struct sockaddr *)&cliaddr,&len);
        strncpy(last_msg, mesg, 8);
        last_id = atoi(last_msg);
        fprintf(out, "%d\n", last_id);
        inst++;
        n=sendto(sockfd,last_msg,strlen(last_msg),0,(struct sockaddr *)&cliaddr,len);
    }

    fclose(out);
    pthread_exit(&last_id);
    return NULL;
}

int main(int argc, char**argv)
{
    int i, err;
    int *ptr[argc-1];
    pthread_t tid[argc-1]; // this is thread identifier


    if (argc < 2) { 
        printf("Usage: ./server [eth*]\n"); 
        exit(1);
    }

    for(i = 0; i < argc-1; i++) {
        err = pthread_create(&tid[i], NULL, recvFunc, argv[i+1]);
        if (err != 0) {
            perror("Thread create Error");
            exit(1);
        }
    }


    for(i = 0; i < argc-1; i++) {
        printf("wait thread %d\n", i);
       pthread_join(tid[i], NULL); 
    }

    return 0;
}
