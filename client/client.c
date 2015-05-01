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
    char buffer[256];
    struct timespec tstart={0,0}, tend={0,0}, res;
    int rc; 
    rc = clock_getres(CLOCK_MONOTONIC, &res);
    if (!rc)
        printf("CLOCK_MONOTONIC: %ldns\n", res.tv_nsec);
    rc = clock_getres(CLOCK_MONOTONIC_COARSE, &res);
        printf("CLOCK_MONOTONIC_COARSE: %ldns\n", res.tv_nsec);

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
    req.tv_sec = 0;
    req.tv_nsec = milisec * 1.0e6;
    
    for(i=1; i < 10; i++) {
        bzero(buffer, 256);
        sprintf(buffer, "%08d\n", i);
        clock_gettime(CLOCK_MONOTONIC_COARSE, &tstart);
        n = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&server, length);
        nanosleep(&req, (struct timespec *)NULL);
        clock_gettime(CLOCK_MONOTONIC_COARSE, &tend);
        printf("some_long_computation took about %.5f milliseconds\n",
        ((double)tend.tv_sec*1.0e3 + 1.0e-6*tend.tv_nsec) - 
           ((double)tstart.tv_sec*1.0e3 + 1.0e-6*tstart.tv_nsec));
        printf("%08d\n", i);
        if (n < 0) error("sendto");
    }
    //n = recvfrom(sock, buffer, 256, 0, (struct sockaddr *) &from, &length);
    //    //if (n < 0) error("recvfrom");
    //        //write(1, "Got an ack: ",12);
    //            //write(1, buffer, n);
    close(sock);
    return 0;
}

