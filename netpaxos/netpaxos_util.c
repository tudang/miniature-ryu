#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

void bindSocket(int sock, int port) {
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(port);
    bind(sock,(struct sockaddr *)&servaddr,sizeof(servaddr));
}


void bindToDevice(int sock, char *iname) {
#ifdef __MACH__ 

#else
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, 
                            iname, strlen(iname)) < 0) 
    {
        perror("Bind to device\n");
        exit(-1);
    }
#endif 
}

void addMembership(int sockfd, char *group) {
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))<0) {
        perror("setsockopt mreq");
        exit(-2);
    }
}

void setRecBuf(int sock) {
    int rcvbuf = 16777216;
    int rc = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(sock));
    if (rc == -1) {
        perror("setRecBuf");
        exit(-1);
    }
}


int getRecBuf(int sock) {
    int rcvbuf = 0;
    socklen_t size = sizeof(sock);
    int rc = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &size);
    if (rc == -1) {
        perror("setRecBuf");
        exit(-1);
    }
    return rcvbuf;
}
