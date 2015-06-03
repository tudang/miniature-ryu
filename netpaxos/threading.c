/* UDP client in the internet domain */
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
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

#define HOST "192.168.4.91"
#define PORT 8888
#define MAX 1470
#define NPACKET 500000
#define BILLION 1000000000L

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

struct server {
    int socket;
    struct sockaddr_in server;
    unsigned int length;
};

void *sendMsg(void *arg)
{
    fd_set write_fd_set;
    struct server *s = (struct server*) arg;
    int sock = s->socket;
    char buffer[] = "hello";

    FD_ZERO(&write_fd_set);
    FD_SET(sock, &write_fd_set);

    int activity = select(sock+1, NULL, &write_fd_set, NULL, NULL);
    if (activity) {
        if (FD_ISSET(sock, &write_fd_set)) {
            int n = sendto(sock, buffer, strlen(buffer), 0, 
                        (struct sockaddr *)&s->server, s->length);
            printf("send %d bytes\n", n);
        }
    }


    return NULL;
}

void *recvMsg(void *arg)
{
    fd_set read_fd_set;
    struct server *s = (struct server*) arg;
    int sock = s->socket;
    char recvbuf[MAX];
    struct timeval timeout = {5, 0};

    FD_ZERO(&read_fd_set);
    FD_SET(sock, &read_fd_set);

    int activity = select(sock+1, &read_fd_set, NULL, NULL, &timeout);
    if (activity) {
        if(FD_ISSET(sock, &read_fd_set)) {
            int n = recvfrom(sock, recvbuf, MAX, 0, NULL, NULL);
            printf("recv %d bytes\n", n);
        }
    }
    else printf("timeout after 5 seconds\n");
    return NULL;
}

int main(int argc, char **argv) 
{
    pthread_t sth, rth; // thread identifier
    struct sockaddr_in server_addr;
    struct hostent *hp;
    unsigned int length;
    struct server *serv;

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
    hp = gethostbyname(HOST);
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
    pthread_create(&sth, NULL, sendMsg, (void*) serv);
    pthread_create(&rth, NULL, recvMsg, (void*) serv);

    /* wait for our thread to finish before continuing */
    pthread_join(sth, NULL);
    pthread_join(rth, NULL);

    printf("Main exit.\n"); 

    return 0;
}
