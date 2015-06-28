#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdio.h>

#define PORT 8888
#define SIZE 1470
#define VAL_LEN 8
#define MAX_INST 500000

int main(int argc, char**argv)
{
    int sockfd,n;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    int instance;
    char mesg[SIZE];
    char str_value[VAL_LEN];
    int values[MAX_INST];

    sockfd=socket(AF_INET,SOCK_DGRAM,0);

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
        sprintf(str_value, "%.8s", mesg);
        int value = atoi(str_value);
        values[instance++] = value;
    }
}
