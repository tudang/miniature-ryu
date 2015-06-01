/* UDP client in the internet domain */
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

#define MAX 1470
#define NPACKET 500000
#define BILLION 1000000000L

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock, n, i, j;
    unsigned int length;
    struct sockaddr_in server, from;
    struct hostent *hp;
    char buffer[MAX];
    char recvline[MAX];
    struct timeval tstart={0,0}, tend={0,0}, res;
    fd_set writefds, read_fds;
    struct timespec send_tbl[NPACKET] = {1,1};
    
    char last_msg[6];
    char sec[10];
    char nsec[9];
    int last_id = 0;
    long long int total_latency = 0;

    if (argc != 5) { printf("Usage: server port N\n");
                        exit(1);
    }
    int client_id = atoi(argv[4]);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock <0) error("socket");

    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fcntl error");
        exit(1);
    }

    server.sin_family = AF_INET;
    hp = gethostbyname(argv[1]);
    if (hp == NULL) error("Uknown host");

    bcopy((char *)hp->h_addr,
         (char *)&server.sin_addr,
         hp->h_length);
    server.sin_port = htons(atoi(argv[2]));
    length = sizeof(struct sockaddr_in);

    FD_ZERO(&writefds);

    FD_SET(sock, &writefds);
    FD_SET(sock, &read_fds);

    struct timespec req = {0};
    struct timespec tsp = {0,0};
    int micro = 700;
    req.tv_sec = 0;
    req.tv_nsec = micro * 1.0e3;
    long total = 0;
    gettimeofday(&tstart, NULL);
    int N = atoi(argv[3]);
    char* msgid; 
    msgid = (char *) malloc(8);
    int count = 0;
    int previous = -1;
    for(i=0; i < (NPACKET / N); i++) {
        for(j=0; j < N; j++) {
            memset(buffer, '@', MAX);
            clock_gettime(CLOCK_REALTIME, &tsp);
            //printf("%lld.%.9ld\n", (long long)tsp.tv_sec, tsp.tv_nsec);
            sprintf(msgid, "%2d%06d%lld.%.9ld", client_id, count, 
                                 (long long) tsp.tv_sec, tsp.tv_nsec);
            send_tbl[count] = tsp;
            //printf("send_tbl[%d]:%lld.%.9ld\n", count, (long long) send_tbl[count].tv_sec, send_tbl[count].tv_nsec);
            count++;
            strncpy(buffer, msgid, 28);
            int activity = select(sock+1, NULL, &writefds, NULL, NULL);
            if (activity) {
                if (FD_ISSET(sock, &writefds)) {
                    n = sendto(sock, buffer, strlen(buffer), 0, 
                            (struct sockaddr *)&server, length);
                    if (n < 0) error("sendto");
                    total += n;
                }
                if (FD_ISSET(sock, &read_fds)) {
                    n = recvfrom(sock,recvline,MAX,0,NULL,NULL);
                    //printf("%s\n", recvline);
                    strncpy(last_msg, recvline+2, 6);
                    //strncpy(sec, recvline+8, 11);
                    //strncpy(nsec, recvline+20, 9);
                    //printf("%s %s %s\n", last_msg, sec, nsec);
                    last_id = atoi(last_msg);
                    if (last_id == previous) continue;
                    previous = last_id;
                    //printf("last_id:%d\n", last_id);
                    struct timespec end;
                    clock_gettime(CLOCK_REALTIME, &end);
                    //printf("End:%lld.%.9ld\n", (long long) end.tv_sec, end.tv_nsec);
                    //printf("Srt:%lld.%.9ld\n", (long long) send_tbl[last_id].tv_sec, send_tbl[last_id].tv_nsec);
                    uint64_t diff = BILLION * (end.tv_sec - send_tbl[last_id].tv_sec) +
                                    end.tv_nsec - send_tbl[last_id].tv_nsec;
                    //printf("%6d\t%llu us\n", last_id, (long long unsigned int) diff / 2000);
                    total_latency += (diff / 2000);
                }
            }
        }
        nanosleep(&req, (struct timespec *)NULL);
    }
    gettimeofday(&tend, NULL);
    timersub(&tend, &tstart, &res);
    double duration = res.tv_sec + res.tv_usec*1.0e-6;
    printf("Duration: %.6f\n", duration);
    printf("Throughput: %.2f\n", ((double)total * 8 / duration * 1.0e-6));
    printf("Packet/second: %.0f\n", ((double)count) / duration );
    printf("Number of sent msg: %d\n", count);
    printf("Avg. Latency: %3.2f\n", ((float) total_latency / count));
    
    close(sock);
    return 0;
}
