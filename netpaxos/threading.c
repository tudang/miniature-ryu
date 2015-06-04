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
#define NPACKET 500001
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

struct timespec send_tbl[NPACKET] = {1,1};

void *sendMsg(void *arg)
{
    fd_set write_fd_set;
    struct server *s = (struct server*) arg;
    int sock = s->socket;
    char buffer[MAX];

    struct timespec tsp;
    struct timespec req = {0};
    req.tv_sec = 0;
    req.tv_nsec = 1;

    int total = 0;
    int count = 0;
    int client_id = 81;
    char msgid[28];



    memset(buffer, '@', MAX);

    FD_ZERO(&write_fd_set);
    FD_SET(sock, &write_fd_set);

    while (count < NPACKET) {
        int activity = select(sock+1, NULL, &write_fd_set, NULL, NULL);
        if (activity) {
            if (FD_ISSET(sock, &write_fd_set)) {
                // get timestamp and attach to message
                clock_gettime(CLOCK_REALTIME, &tsp);
                sprintf(msgid, "%2d%06d%lld.%.9ld", client_id, count,
                        (long long) tsp.tv_sec, tsp.tv_nsec);
                strncpy(buffer, msgid, 28);
                // put (value,timestamp)
                send_tbl[count] = tsp;
                int n = sendto(sock, buffer, strlen(buffer), 0, 
                            (struct sockaddr *)&s->server, s->length);
                if (n < 0) error("sendto");
                total += n;
                //printf("send %d bytes: [%s]\n", n, msgid);
            }
        }
        count++;

        if ((count % 5) == 0) 
            nanosleep(&req, (struct timespec *)NULL);
    }


    return NULL;
}

void *recvMsg(void *arg)
{
    fd_set read_fd_set;
    struct server *s = (struct server*) arg;
    int sock = s->socket;
    char recvbuf[MAX];
    struct timeval timeout = {30, 0};
    char last_msg[6];
    int last_id;
    long long int total_latency = 0;
    int count = 0;

    FD_ZERO(&read_fd_set);
    FD_SET(sock, &read_fd_set);

    while(1) {
        int activity = select(sock+1, &read_fd_set, NULL, NULL, NULL);
        if (activity) {
            if(FD_ISSET(sock, &read_fd_set)) {
                int n = recvfrom(sock, recvbuf, MAX, 0, NULL, NULL);
                if (n < 0) error("recvfrom");
                strncpy(last_msg, recvbuf+2, 6);
                last_id = atoi(last_msg);
                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);
                uint64_t diff = BILLION * (end.tv_sec - send_tbl[last_id].tv_sec) +
                                    end.tv_nsec - send_tbl[last_id].tv_nsec;
                total_latency += (diff / 2000);
                count++;
                // printf("recv %d bytes: %s\n", n, recvbuf);
            }
        } 
        
        if ((count%100000) == 0)
        {
            printf("Avg. Latency: %ld / %d = %3.2f\n", total_latency, count,
            ((float) total_latency / count));
        }
        
    }


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
    sleep(5);
    pthread_cancel(rth);

    printf("Main exit.\n"); 

    return 0;
}
