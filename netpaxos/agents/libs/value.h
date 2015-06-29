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


#define GROUP "239.0.0.1"
#define PORT 8888
#define MAX_CLIENT  1000000 // client's max sequence
#define BUF_SIZE 1470
#define BILLION 1000000000L
#define MAX_SERVER 1000000 // receiver's max sequence

typedef struct values {
    int client_id;
    int sequence;
    struct timespec ts;
} value;


typedef struct paxosvals {
    int instance;
    int crnd;
    int vrnd;
    int vval;
} paxosval;

void serialize_value(value v, char* buffer);
void deserialize_value(char* buffer, value *v);
uint64_t timediff(struct timespec start, struct timespec end);
paxosval new_value(int inst, int crnd, int vrnd, int vval);
void netpaxos_to_string(char *str, paxosval p); 
