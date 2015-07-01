#include "value.h"

void *recvFunc(void *arg);

int main(int argc, char**argv)
{
    int i, err;
    int *ptr[argc-1];
    pthread_t tid[argc-1]; // this is thread identifier


    if (argc < 2) { 
        printf("Usage: %s [eth*]\n", argv[0]); 
        exit(1);
    }

    for(i = 0; i < argc-1; i++) {
        err = pthread_create(&tid[i], NULL, recvFunc, argv[i+1]);
        if (err != 0) {
            perror("Thread create Error");
            exit(1);
        }
    }


    for(i = 0; i < argc-1; i++) {
        printf("wait thread %d\n", i);
       pthread_join(tid[i], NULL); 
    }

    return 0;
}


/* This is thread function */
void *recvFunc(void *arg)
{
    int sockfd, n;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char mesg[BUF_SIZE];
    char *itf;
    itf = (char*)arg; 
    pthread_t self_id;
    self_id = pthread_self();
    int inst = 1;
    value v; 

    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if (sockfd < 0) {
        error("ERROR opening socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, 
                            itf, strlen(itf)) < 0) 
    {
        perror("Setsockopt Error\n");
        exit(1);    
    } 

 
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);


    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, itf, IFNAMSIZ-1);
    ioctl(sockfd, SIOCGIFADDR, &ifr);
    struct in_addr sin = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
    char itf_addr[INET_ADDRSTRLEN]; 
    if (inet_ntop(AF_INET, &sin, itf_addr, sizeof(itf_addr)) == NULL)
        perror("inet_ntop");
    
    struct ip_mreq mreq;

    mreq.imr_multiaddr.s_addr = inet_addr(GROUP);
    mreq.imr_interface.s_addr = inet_addr(itf_addr);

    if (setsockopt(sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))<0) {
        error("setsockopt mreq");
        exit(1);
    }
    
    if (bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        error("bind");
        exit(1);
    }

    FILE *out;
    char filename[20];
    sprintf(filename, "/tmp/%s.capture", itf);
    out = fopen(filename, "w");

    if (out == NULL) {
        perror("Error opening file");
        exit(1);
    }

    char buf[BUF_SIZE];
    len = sizeof(cliaddr);

    do {
        value v;
        n=recvfrom(sockfd, &v,sizeof(value),0,(struct sockaddr *)&cliaddr,&len);
        struct header h = v.header;
        fprintf(out, "%2.d%.6d\n", h.client_id, h.sequence);
        char hdr[40];
        //header_to_string(hdr, h);
        //fprintf(stdout, "%.1d,%.2d,%.6d,%.d,%s\n", h.msg_type, h.client_id, 
        //                                            h.sequence, h.buffer_size, v.buffer);
        
        inst++;
        int seq = htonl(h.sequence);
        n=sendto(sockfd,&seq,sizeof(seq),0,(struct sockaddr *)&cliaddr,len);
    } while (inst < MAX_SERVER - 1);

    fclose(out);
    pthread_exit(NULL);
    return NULL;
}
