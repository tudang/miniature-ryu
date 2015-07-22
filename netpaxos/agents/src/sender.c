/* UDP client in the internet domain */
#include "value.h"
int put(char *key, char *value);
int update(char *key, char *value);
int get(char *key);
int delete(char *key);

void usage(char *program);
void parseArguments(int argc, char **argv, int *t, int *N, int *id);
void *recvMsg(void *arg);
struct timespec submitValue(int sock, char* str, int len, int nid, int count);
struct timespec send_tbl[MAX_CLIENT] = {1,1};
int submitValues(int sock, char *str, int len, int N, int nid, struct timespec req);
static void random_string(char *s, const int len);

int main(int argc, char **argv) {
    pthread_t sth, rth; // thread identifier
    struct sockaddr_in local;
    struct ip_mreq mreq;
    struct hostent *he;
    unsigned int length;
    int t = 1; // Number of nanoseconds to sleep
    int N = 1; // Number of message sending every t ns
    int nid = 1; // node_id
    struct timespec tstart, tend;
    struct timespec req = {0, t};

    if (argc < 2) {
        usage(argv[0]);
        exit(1);
    }
    parseArguments(argc, argv, &t, &N, &nid);
    char *myniccard = argv[optind];
    char *itf_addr = get_interface_addr(myniccard);
    /* Create socket */
    int sock;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("socket");
    //if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) error("fcntl error");
    int ttl = 1; 
    setsockopt (sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(itf_addr);
    local.sin_port = htons(PORT);
    bind(sock, (struct sockaddr *)&local, sizeof(local));
    /* Create worker thread */
    pthread_create(&rth, NULL, recvMsg, (void*)&sock);
    // get time start sending
    clock_gettime(CLOCK_REALTIME, &tstart);
    char *sample = malloc(VALUE_SIZE);
    random_string(sample, VALUE_SIZE);
    int count = submitValues(sock, sample, strlen(sample), N, nid, req); 
    // get time end sending
    clock_gettime(CLOCK_REALTIME, &tend);
    float duration = timediff(tstart, tend) / BILLION;
    printf("packets/second: %3.2f\n", (float) count / duration);
    printf("Mbps/second: %3.2f\n", (float) count * 8 * (VALUE_SIZE + 42) / 1000000 / duration);
    /* wait for our thread to finish before continuing */
    sleep(5);
    pthread_cancel(rth);
    printf("Main exit.\n"); 
    return 0;
}

void *recvMsg(void *arg) {
    int sock = *((int*) arg);
    char recvbuf[BUF_SIZE];
    struct timeval timeout = {30, 0};
    char last_msg[6];
    int last_id;
    long long int total_latency = 0;
    int count = 0;
    while(1) {
        int seq = 0;
        int n = recvfrom(sock, &seq, sizeof(seq), 0, NULL, NULL);
        if (n < 0) error("recvfrom");
        int current_id = ntohl(seq);
        //printf("last_id:%d\n", last_id);
        if (current_id == last_id) continue;
        last_id = current_id;
        struct timespec end;
        clock_gettime(CLOCK_REALTIME, &end);
        uint64_t diff = timediff(send_tbl[last_id], end);
        total_latency += (diff / 2000);
        count++;
        // if ((count%100000) == 0) 
        if ((count%100) == 0) { // For DEBUG
            printf("Avg. Latency: %ld / %d = %3.2f us\n", total_latency, count,
            ((float) total_latency / count));
        }
        
    }
}

void usage(char *program) {
    printf("Usage: %s [OPTIONS] interface\n \
    options:\n \
    -c N client id\n \
    -n N number of packet per nanosecond\n \
    -t number of nanosecond to sleep between two send\n", 
    program);
}

void parseArguments(int argc, char **argv, int *t, int *N, int *id) {
    int c;
    while  ((c = getopt (argc, argv, "n:t:c:")) != -1) {
        switch(c)
        {
            case 'n':
                *N = atoi(optarg);
                break;

            case 't':
                *t = atoi(optarg);
                break;

            case 'c':
                *id = atoi(optarg);
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
                  fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                exit(1);
            default:
                exit(1);
        }
    }
    /*
    int index;
    for (index = optind; index < argc; index++)
        printf ("Non-option argument %d %s\n", index, argv[index]);
    */
}

struct timespec submitValue(int sock, char* str, int len, int nid, int count) {
    struct timespec tsp;
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(GROUP);
    server.sin_port = htons(PORT);
    size_t length = sizeof(struct sockaddr_in);
    value v;
    clock_gettime(CLOCK_REALTIME, &tsp);
    struct header h;
    h.msg_type = ACCEPT;
    h.nid = nid;
    h.instance = count;
    h.round = 1;
    h.vround = 0;
    h.key = (count << 4) + nid;
    h.buffer_size = len;
    v.header = h;
    strncpy(v.value, str, len);
    size_t msize = sizeof(struct header) + h.buffer_size;
    int n = sendto(sock, &v, msize, 0, 
                (struct sockaddr *)&server, length);
    if (n < 0) error("sendto");
    send_tbl[count] = tsp;
    return tsp;
}

int submitValues(int sock, char *str, int len, int N, int nid, struct timespec req) {
    int count = 1;
    while (count < MAX_CLIENT) {
        submitValue(sock, str, len, nid, count); 
        count++;
        if ((count % N) == 0) 
            nanosleep(&req, (struct timespec *)NULL);
    }
    return count;
}

static void random_string(char *s, const int len)
{
    int i;
    static const char alphanum[] =
        "0123456789abcdefghijklmnopqrstuvwxyz";
    for (i = 0; i < len-1; ++i)
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    s[len-1] = 0;
}
