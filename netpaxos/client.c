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

#define MAX 1470
#define NPACKET 500000

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
    struct timeval tstart={0,0}, tend={0,0}, res;
    fd_set writefds;

    if (argc != 5) { printf("Usage: server port N\n");
                        exit(1);
    }
    int client_id = atoi(argv[4]);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock <0) error("socket");

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
    for(i=0; i < (NPACKET / N); i++) {
        for(j=0; j < N; j++) {
            memset(buffer, '@', MAX);
            clock_gettime(CLOCK_REALTIME, &tsp);
            //printf("%lld.%.9ld\n", (long long)tsp.tv_sec, tsp.tv_nsec);
            sprintf(msgid, "%2d%06d%lld.%.9ld", client_id, count++, 
                                 (long long) tsp.tv_sec, tsp.tv_nsec);
            strncpy(buffer, msgid, 28);
            int activity = select(sock+1, NULL, &writefds, NULL, NULL);
                if (activity) {
                    if (FD_ISSET(sock, &writefds)) {
                n = sendto(sock, buffer, strlen(buffer), 0, 
                            (struct sockaddr *)&server, length);
                if (n < 0) error("sendto");
                total += n;
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
    close(sock);
    return 0;
}
