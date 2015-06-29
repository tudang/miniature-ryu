/* UDP client in the internet domain */
#include "value.h"

void error(const char *msg);
void *recvMsg(void *arg);
uint64_t timediff(struct timespec start, struct timespec end);

struct server {
    int socket;
    struct sockaddr_in server;
    unsigned int length;
};


void serialize_value(value v, char* buffer) {
    int MSG_SIZE = 28; // size of the output string in sprintf below 
    char msgid[MSG_SIZE]; 
    sprintf(msgid, "%2d%06d%lld.%.9ld", v.client_id, v.sequence,
                        (long long) v.ts.tv_sec, v.ts.tv_nsec);
    strncpy(buffer, msgid, MSG_SIZE);
}

struct timespec send_tbl[MAX_CLIENT] = {1,1};

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

uint64_t timediff(struct timespec start, struct timespec end)
{
    return (BILLION * (end.tv_sec - start.tv_sec) +
                    end.tv_nsec - start.tv_nsec);
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
    char *myniccard;
    value v;

    serv = malloc(sizeof(struct server));

    while  ((c = getopt (argc, argv, "n:t:c:i:")) != -1) {
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
            
            case 'i':
                myniccard = optarg;
                break;

            default:
                exit(1);
        }
    }
    
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
    char buffer[BUF_SIZE];

    struct timespec tsp, tstart, tend;
    struct timespec req = {0};
    req.tv_sec = 0;
    req.tv_nsec = t;

    int total = 0;
    int count = 0;

    memset(buffer, '@', BUF_SIZE);

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
                v.client_id = client_id;
                v.sequence = count;
                v.ts = tsp;
                serialize_value(v, buffer);
                // put (value,timestamp)
                send_tbl[count] = tsp;
                int n = sendto(sock, buffer, strlen(buffer), 0, 
                            (struct sockaddr *)&server, length);
                if (n < 0) error("sendto");
                total += n;
                //printf("send %d bytes: [%d]\n", n, v.sequence);
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
