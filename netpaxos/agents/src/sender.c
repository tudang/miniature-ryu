/* UDP client in the internet domain */
#include "value.h"

void error(const char *msg);
void *recvMsg(void *arg);

struct server {
    int socket;
    struct sockaddr_in server;
    unsigned int length;
};


struct timespec send_tbl[MAX_CLIENT] = {1,1};

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void *recvMsg(void *arg)
{
    fd_set read_fd_set;
    struct server *s = (struct server*) arg;
    int sock = s->socket;
    char recvbuf[BUF_SIZE];
    struct timeval timeout = {30, 0};
    char last_msg[6];
    int last_id;
    long long int total_latency = 0;
    int count = 0;

    FD_ZERO(&read_fd_set);
    FD_SET(sock, &read_fd_set);

    while(1) {
        int activity = select(sock+1, &read_fd_set, NULL, NULL, NULL);
        if (activity) {
            if(FD_ISSET(sock, &read_fd_set)) {
                int seq = 0;
                int n = recvfrom(sock, &seq, sizeof(seq), 0, NULL, NULL);
                if (n < 0) error("recvfrom");
                last_id = ntohl(seq);
                //printf("last_id:%d\n", last_id);
                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);
                uint64_t diff = timediff(send_tbl[last_id], end);
                total_latency += (diff / 2000);
                count++;
            }
        } 
        
        if ((count%100000) == 0)
        {
            printf("Avg. Latency: %ld / %d = %3.2f us\n", total_latency, count,
            ((float) total_latency / count));
        }
        
    }


    return NULL;
}


int main(int argc, char **argv) 
{
    pthread_t sth, rth; // thread identifier
    struct sockaddr_in local, server;
    struct ip_mreq mreq;
    struct hostent *he;
    unsigned int length;
    struct server *serv;
    int c;
    int t = 1; // Number of nanoseconds to sleep
    int N = 1; // Number of message sending every t ns
    int client_id = 81; // Client id
    value v;

    serv = malloc(sizeof(struct server));

    while  ((c = getopt (argc, argv, "n:t:c:")) != -1) {
        switch(c)
        {
            case 'n':
                N = atoi(optarg);
                break;

            case 't':
                t = atoi(optarg);
                break;

            case 'c':
                client_id = atoi(optarg);
                break;

            case '?':
                if (optopt == 'n')
                  fprintf (stderr, "Option -%n requires an argument.\n", optopt);
                if (optopt == 't')
                  fprintf (stderr, "Option -%t requires an argument.\n", optopt);
                if (optopt == 'c')
                  fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                  fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                  fprintf (stderr,
                           "Unknown option character `\\x%x'.\n",
                           optopt);
                return 1;

            default:
                exit(1);
        }
    }

    /*
    int index;
    for (index = optind; index < argc; index++)
        printf ("Non-option argument %d %s\n", index, argv[index]);
    */
    
    if (optind < argc-2) {
        printf("Usage: %s [OPTIONS] interface\n \
                options:\n \
                -c N client id\n \
                -n N number of packet per nanosecond\n \
                -t number of nanosecond to sleep between two send\n", 
                argv[0]);
        exit(1);
    }
    char *myniccard = argv[optind];
    
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
      ifr.ifr_addr.sa_family = AF_INET;

      /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, myniccard, IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    /* display result */
    char *itf_addr = 
     inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    printf("MY IP address:%s: on port: %d\n", itf_addr, PORT);
    
    /* Create socket */
    int sock;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("socket");
    
    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
        error("fcntl error");
    
    
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(itf_addr);
    local.sin_port = htons(PORT);
    bind(sock, (struct sockaddr *)&local, sizeof(local));
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(GROUP);
    server.sin_port = htons(PORT);
    length = sizeof(struct sockaddr_in);

    (*serv).socket = sock;
    (*serv).server = server;
    (*serv).length = length;


    /* Create worker thread */
    pthread_create(&rth, NULL, recvMsg, (void*) serv);

    /* Sending in Main thread */
    fd_set write_fd_set;
    
    // unsigned char buffer[BUF_SIZE];
    // memset(buffer, '@', BUF_SIZE);

    struct timespec tsp, tstart, tend;
    struct timespec req = {0};
    req.tv_sec = 0;
    req.tv_nsec = t;

    int total = 0;
    int count = 1;


    FD_ZERO(&write_fd_set);
    FD_SET(sock, &write_fd_set);

    // get time start sending
    clock_gettime(CLOCK_REALTIME, &tstart);

    while (count < MAX_CLIENT) {
        int activity = select(sock+1, NULL, &write_fd_set, NULL, NULL);
        if (activity) {
            if (FD_ISSET(sock, &write_fd_set)) {
                // get timestamp and attach to message
                clock_gettime(CLOCK_REALTIME, &tsp);
                struct header h;
                h.msg_type = ACCEPT;
                h.client_id = client_id;
                h.sequence = count;
                h.ts = tsp;
                h.buffer_size = 1430;
                v.header = h;
            
                strncpy(v.buffer, "hello NetPaxos", h.buffer_size);
                send_tbl[count] = tsp;
                size_t msize = sizeof(struct header) + h.buffer_size;

                int n = sendto(sock, &v, msize, 0, 
                            (struct sockaddr *)&server, length);
                if (n < 0) error("sendto");
                total += n;
            }
        }
        count++;


        if ((count % N) == 0) 
            nanosleep(&req, (struct timespec *)NULL);
    }
    // get time end sending
    clock_gettime(CLOCK_REALTIME, &tend);
    float duration = timediff(tstart, tend) / BILLION;
    printf("packets/second: %3.2f\n", (float) count / duration);
    /* wait for our thread to finish before continuing */
    sleep(5);
    pthread_cancel(rth);

    printf("Main exit.\n"); 

    return 0;
}
