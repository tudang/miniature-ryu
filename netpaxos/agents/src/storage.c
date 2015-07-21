#include "value.h"


void *recovery_handler(void *arg);

int main(int argc, char**argv)
{

    if (argc < 2) {
        printf("%s output-file\n", argv[0]);
        exit(1);
    }

    int n;
    struct sockaddr_in cliaddr;
    socklen_t len;
    int instance;
    int values[MAX_SERVER];
    paxosval paxos_value;
    FILE *out;
    char str_pval[20];

    out = fopen(argv[1], "w+");
    if (out == NULL) {
        perror("Error opening file");
        exit(1);
    }
    
    int sockfd = create_multicast_socket(GROUP, PORT);

    pthread_t recover_thread;
    int recover_sockfd = create_multicast_socket(GROUP, RECOVER_PORT);
    int *thread_arg = malloc(sizeof(int));
    *thread_arg = recover_sockfd;
    pthread_create(&recover_thread, NULL, recovery_handler, thread_arg);
    
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


int create_multicast_socket(char *group, int port) {

    struct sockaddr_in servaddr;
    struct ip_mreq mreq;

    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if (sockfd < 0) {
        error("ERROR opening socket");
        exit(1);
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(port);

    mreq.imr_multiaddr.s_addr = inet_addr(group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))<0) {
        error("setsockopt mreq");
        exit(1);
    }
    
    if (bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        error("bind");
        exit(1);
    }
    
    printf("Listening on port: %d \n", port);
    return sockfd;
}


void *recovery_handler(void *arg) {
    printf("Successfully start recovery thread\n");
    int sock = *((int*) arg);
    struct sockaddr_in cliaddr;
    socklen_t len;
    value v;
    int n;
    for (;;)
    {
        len = sizeof(cliaddr);
        n = recvfrom(sock,&v,sizeof(value),0,(struct sockaddr *)&cliaddr,&len);
    }
}
