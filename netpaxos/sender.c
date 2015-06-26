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
#include <net/if.h>
#include <sys/ioctl.h>

#define GROUP "239.0.0.1"
#define PORT 8888
#define MAX 1470
#define NPACKET 500001
#define BILLION 1000000000L

struct server {
    int socket;
    struct sockaddr_in server;
    unsigned int length;
};

struct timespec send_tbl[NPACKET] = {1,1};

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


void my_ip( char *myniccard, char *myipaddr) {
}   

int main(int argc, char **argv) 
{
    pthread_t sth, rth; // thread identifier
    struct sockaddr_in local, server;
    struct ip_mreq mreq;
    struct hostent *he;
    unsigned int length;
    struct server *serv;
    int c;
    int t = 1; // Number of nanoseconds to sleep
    int N = 1; // Number of message sending every t ns
    int client_id = 81; // Client id
    char *myniccard;

    serv = malloc(sizeof(struct server));

    while  ((c = getopt (argc, argv, "n:t:c:a:")) != -1) {
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
            
            case 'a':
                myniccard = optarg;
                break;

            default:
                error("missing arguments");
        }
    }
    
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
      ifr.ifr_addr.sa_family = AF_INET;

      /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, myniccard, IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    /* display result */
    char *itf_addr = 
     inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    printf("MY IP address:%s: on port: %d\n", itf_addr, PORT);
    
    /* Create socket */
    int sock;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("socket");
    
    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fcntl error");
        exit(1);
    }
    
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(itf_addr);
    local.sin_port = htons(PORT);
    bind(sock, (struct sockaddr *)&local, sizeof(local));
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(GROUP);
    server.sin_port = htons(PORT);
    length = sizeof(struct sockaddr_in);

    (*serv).socket = sock;
    (*serv).server = server;
    (*serv).length = length;


    /* Create worker thread */
    pthread_create(&rth, NULL, recvMsg, (void*) serv);

    /* Sending in Main thread */
    fd_set write_fd_set;
    char buffer[MAX];

    struct timespec tsp, tstart, tend;
    struct timespec req = {0};
    req.tv_sec = 0;
    req.tv_nsec = t;

    int total = 0;
    int count = 0;
    char msgid[28];

    memset(buffer, '@', MAX);

    FD_ZERO(&write_fd_set);
    FD_SET(sock, &write_fd_set);

    // get time start sending
    clock_gettime(CLOCK_REALTIME, &tstart);

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
                            (struct sockaddr *)&server, length);
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
    /* wait for our thread to finish before continuing */
    sleep(5);
    pthread_cancel(rth);

    printf("Main exit.\n"); 

    return 0;
}
