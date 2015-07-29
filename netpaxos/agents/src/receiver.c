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
    int n;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char mesg[BUF_SIZE];
    char *itf;
    itf = (char*)arg; 
    pthread_t self_id;
    self_id = pthread_self();
    int inst = 1;
    value v; 

    //int sockfd = newInterfaceBoundSocket(itf);
    int sockfd = newUDPSocket();

    int yes = 1;
    if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
    {
        perror("setsockopt");
    }
 
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);


    char *itf_addr = get_interface_addr(itf);
    addMembership(&sockfd, GROUP, itf_addr);
  
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
        char hdr[40];
        header_to_string(hdr, h);
        printf("%s\n", hdr);
        inst++;
        int seq = htonl((h.key >> 4));
        n=sendto(sockfd,&seq,sizeof(seq),0,(struct sockaddr *)&cliaddr,len);
    } while (inst < MAX_SERVER - 1);

    fclose(out);
    pthread_exit(NULL);
    return NULL;
}
