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
#include <ctype.h>

#include "config.h"

struct server {
    int socket;
    struct sockaddr_in server;
    unsigned int length;
};

struct timespec send_tbl[MAX_NUM] = {1,1};

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

uint64_t timediff(struct timespec start, struct timespec end)
{
    return (BILLION * (end.tv_sec - start.tv_sec) +
                    end.tv_nsec - start.tv_nsec);
}


void *recvMsg(void *arg)
{
    fd_set read_fd_set;
    struct server *s = (struct server*) arg;
    int sock = s->socket;
    char recvbuf[BUF_SIZE];
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
                int n = recvfrom(sock, recvbuf, BUF_SIZE, 0, NULL, NULL);
                if (n < 0) error("recvfrom");
                strncpy(last_msg, recvbuf+2, 6);
                last_id = atoi(last_msg);
                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);
                uint64_t diff = timediff(send_tbl[last_id], end);
                total_latency += (diff / 2000);
                count++;
                // printf("recv %d bytes: %s\n", n, recvbuf);
            }
        } 
        
        if ((count%100000) == 0)
        {
            printf("Avg. Latency: %ld / %d = %3.2f us\n", total_latency, count,
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
    int c;
    int t = 1; // Number of nanoseconds to sleep
    int N = 1; // Number of message sending every t ns
    int client_id = 81; // Client id

    serv = malloc(sizeof(struct server));

    while  ((c = getopt (argc, argv, "n:t:c:")) != -1) {
        switch(c)
        {
            case 'n':
                N = atoi(optarg);
                break;

            case 't':
                t = atoi(optarg);
                break;

            case 'c':
                client_id = atoi(optarg);
                break;

            default:
                error("missing arguments");
        }
    }
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
    hp = gethostbyname(SERVER);
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
    pthread_create(&rth, NULL, recvMsg, (void*) serv);

    /* Sending in Main thread */
    fd_set write_fd_set;
    char buffer[BUF_SIZE];

    struct timespec tsp, tstart, tend;
    struct timespec req = {0};
    req.tv_sec = 0;
    req.tv_nsec = t;

    int total = 0;
    int count = 0;
    char msgid[28];

    memset(buffer, '@', BUF_SIZE);

    FD_ZERO(&write_fd_set);
    FD_SET(sock, &write_fd_set);

    // get time start sending
    clock_gettime(CLOCK_REALTIME, &tstart);

    while (count < (MAX_NUM*0.51)) {
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
                            (struct sockaddr *)&server_addr, length);
                if (n < 0) error("sendto");
                total += n;
                //printf("send %d bytes: [%s]\n", n, msgid);
            }
        }
        count++;

        if ((count % N) == 0) 
            nanosleep(&req, (struct timespec *)NULL);
    }
    // get time end sending
    clock_gettime(CLOCK_REALTIME, &tend);
    float duration = timediff(tstart, tend) / BILLION;

    printf("packets/second: %3.2f\n", (float) count / duration);
    printf("Total packets/second: %3.2f\n", ((float) count / duration) * 2);
    /* wait for our thread to finish before continuing */
    sleep(5);
    pthread_cancel(rth);
    return 0;
}
