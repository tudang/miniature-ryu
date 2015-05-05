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

#define MAX 1470

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock, n, i;
    unsigned int length;
    struct sockaddr_in server, from;
    struct hostent *hp;
    char buffer[MAX];
    struct timespec tstart={0,0}, tend={0,0}, res;

    if (argc != 3) { printf("Usage: server port\n");
                        exit(1);
    }
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
    struct timespec req = {0};
    int milisec = 1;
    int micro = 1;
    req.tv_sec = 0;
    req.tv_nsec = micro * 1.0e3;
    long total;
    clock_gettime(CLOCK_REALTIME, &tstart);
    int npacket=1.0e6;
    for(i=0; i < npacket; i++) {
        memset(buffer, '@', 1470);
        char msgid[8]; 
        sprintf(msgid, "%08d", i);
        strncpy(buffer, msgid, 8);
        clock_gettime(CLOCK_REALTIME, &tstart);
        n = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&server, length);
        if (n < 0) error("sendto");
        total += n;
        //nanosleep(&req, (struct timespec *)NULL);
        clock_gettime(CLOCK_REALTIME, &tend);
        double duration = ((double)tend.tv_sec*1E9 + tend.tv_nsec) - 
        ((double)tstart.tv_sec*1E9 + tstart.tv_nsec);
        printf("some_long_computation took about %.5f nanoseconds\n", duration);
    }
    //fwrite(buffer, 16, 1, stdout);
    clock_gettime(CLOCK_REALTIME, &tend);
    //printf("strlen(buffer): %d\n", strlen(buffer));
    double duration = ((double)tend.tv_sec*1.0e3 + 1.0e-6*tend.tv_nsec) - 
    ((double)tstart.tv_sec*1.0e3 + 1.0e-6*tstart.tv_nsec);
    printf("some_long_computation took about %.5f milliseconds\n", duration);
    printf("Total bytes sent %d\n", total);
    printf("Throughput: %.2f\n", ((double)total * 8 / duration * 1.0e-6));
    //n = recvfrom(sock, buffer, 256, 0, (struct sockaddr *) &from, &length);
    //    //if (n < 0) error("recvfrom");
    //        //write(1, "Got an ack: ",12);
    //            //write(1, buffer, n);
    close(sock);
    return 0;
}

