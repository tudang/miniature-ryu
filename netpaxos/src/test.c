#include <stdio.h>
#include <unistd.h>
#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>


#define GROUP "239.0.0.1"
#define PORT 8888

int main(int argc, char* argv[])
{
    int fd;
    struct ifreq ifr;

    char *itf_name = argv[1];

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, itf_name, IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    /* display result */
    char *sin_addr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    printf("%s\n", sin_addr);

    struct sockaddr_in addr;
    int addrlen;
    
    bzero((char *)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    addrlen = sizeof(addr);

    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, 
                            itf_name, strlen(itf_name)) < 0) 
    {
        perror("Setsockopt Error\n");
        exit(1);    
    } 

    if (bind(fd, (struct sockaddr *)&addr, addrlen) < 0) {
        perror("bind");
        exit(1);
    }

    struct ip_mreq mreq;
    char message[50];
    int cnt = 0;

    mreq.imr_multiaddr.s_addr = inet_addr(GROUP);
    mreq.imr_interface.s_addr = inet_addr(sin_addr);
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt mreq");
        exit(1);
    }

    int so_reuseaddr = 1;
    int status = 0;
    status = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, 
                &so_reuseaddr, sizeof so_reuseaddr);
    if (status < 0) {
        perror("setsockopt");
        exit(1);
    }
    
    
    while(1) {
        cnt = recvfrom(fd, message, sizeof(message), 0,
                (struct sockaddr *)&addr, &addrlen);
        if (cnt < 0) {
            perror("recvfrom");
            exit(1);
        }
        else if (cnt == 0) 
            break;
    
        printf("%s: message = \"%s\"\n", inet_ntoa(addr.sin_addr), message);
    }
    



 return 0;
}
