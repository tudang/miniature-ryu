#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 8888
#define SIZE 1470
int main(int argc, char**argv)
{
    int sockfd,n;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    char mesg[SIZE];
 
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, 
                            argv[1], strlen(argv[1])) < 0) 
    {
        perror("Setsockopt Error\n");
        exit(1);    
    } 
 
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);
    bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
 
    for (;;)
    {
       len = sizeof(cliaddr);
       n = recvfrom(sockfd,mesg,SIZE,0,(struct sockaddr *)&cliaddr,&len);
       //sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
       printf("%.8s\n",mesg);
    }
}
