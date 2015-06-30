#include "value.h"

#define VAL_LEN 9

int main(int argc, char**argv)
{

    if (argc < 2) {
        printf("%s output-file\n", argv[0]);
        exit(1);
    }

    int sockfd,n;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    int instance;
    char mesg[BUF_SIZE];
    char str_value[VAL_LEN];
    int values[MAX_SERVER];
    struct ip_mreq mreq;
    value v;
    paxosval paxos_value;
    FILE *out;
    char str_pval[20];

    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if (sockfd < 0) {
        error("ERROR opening socket");
        exit(1);
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);

    mreq.imr_multiaddr.s_addr = inet_addr(GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))<0) {
        error("setsockopt mreq");
        exit(1);
    }
    
    if (bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        error("bind");
        exit(1);
    }
    
    printf("listening on port: %d \n", PORT);

    out = fopen(argv[1], "w+");
    if (out == NULL) {
        perror("Error opening file");
        exit(1);
    }
    
    for (;;)
    {
        len = sizeof(cliaddr);
        n = recvfrom(sockfd,mesg,BUF_SIZE,0,(struct sockaddr *)&cliaddr,&len);
        //sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
        deserialize_value(mesg, &v);
        values[instance] = v.sequence;
        paxos_value = new_value(instance++, 1, 1, v.sequence);
        netpaxos_to_string(str_pval, paxos_value);
        fprintf(out, "%s", str_pval);
    }

    fclose(out);
}
