#include "value.h"

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
    int values[MAX_SERVER];
    struct ip_mreq mreq;
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
    
    printf("Listening on port: %d \n", PORT);

    out = fopen(argv[1], "w+");
    if (out == NULL) {
        perror("Error opening file");
        exit(1);
    }
    
    for (;;)
    {
        len = sizeof(cliaddr);
        value v;
        n = recvfrom(sockfd,&v,sizeof(value),0,(struct sockaddr *)&cliaddr,&len);
        struct header h = v.header;
         switch (h.msg_type) {
            case PREPARE:
                break;
            case PROMISE:
                break;
            case ACCEPT:
                values[instance] = h.sequence;
                paxos_value = new_value(instance++, 1, 1, h.sequence);
                netpaxos_to_string(str_pval, paxos_value);
                fprintf(out, "%s", str_pval);
                // response to sender
                int seq = htonl(h.sequence);
                n = sendto(sockfd,&seq,sizeof(seq), 0, (struct sockaddr *)&cliaddr, len);
                if (n < 0) error("sendto");
                break;
            case ACCEPTED:
                break;
            default:
                printf("Error unusual case\n");
                break;
        }
        if (instance % 100000 == 0) printf("Received %d values\n", instance);
    }

    fclose(out);
}
