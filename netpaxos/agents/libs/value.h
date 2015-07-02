#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <ctype.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <errno.h>


#define GROUP "239.0.1.2"
#define PORT 8888
#define MAX_CLIENT  1000000 - 1// client's max sequence
#define BUF_SIZE 1470
#define BILLION 1000000000L
#define MAX_SERVER 1000000 - 1 // receiver's max sequence
#define VALUE_SIZE 1430

enum  message_t {
    PREPARE,
    PROMISE,
    ACCEPT,
    ACCEPTED
};

struct header {
    int msg_type;
    int client_id;
    int sequence;
    struct timespec ts;
    int buffer_size;
};

typedef struct values {
    struct header header;
    char buffer[VALUE_SIZE];
} value;


typedef struct paxosvals {
    int instance;
    int crnd;
    int vrnd;
    int vval;
} paxosval;

uint64_t timediff(struct timespec start, struct timespec end);
paxosval new_value(int inst, int crnd, int vrnd, int vval);
void netpaxos_to_string(char *str, paxosval p); 
void header_to_string(char *str, struct header hd); 
