#include "value.h"

pthread_t tid[4]; // this is thread identifier
int values[4][MAX_SERVER];  // value queues
int start_receiving = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;
int cont = 1; // share variable between main & eval thread
int printarray = 1;

char* kv[MAX_SERVER];

void init_kv() {
    int i;
    for (i=0; i < MAX_SERVER; i++) {
        kv[i] = malloc(VALUE_SIZE);
    }
}

void put(int key, char* value, int size) {
    strncpy(kv[key], value, size);
}


void update(int key, char* value, int size) {
    strncpy(kv[key], value, size);
}

char *get(int key) {
    return kv[key];
}

void delete(int key) {
    memset(kv[key],'\0', strlen(kv[key]));
}


struct thread_args {
    int index;
    char *itf;
};

/* This is thread function */
void *recvFunc(void *arg)
{
    int sockfd, n;
    struct sockaddr_in servaddr,cliaddr;
    socklen_t len;
    struct thread_args *itf;
    itf = (struct thread_args*)arg; 
    int index = itf->index;
    int inst = 0;
    pthread_t self_id;
    self_id = pthread_self();
    sockfd =  newInterfaceBoundSocket(itf->itf);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);
    char *itf_addr = get_interface_addr(itf->itf);
    addMembership(&sockfd, GROUP, itf_addr); 
    if (bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        error("bind");
        exit(1);
    }
    len = sizeof(cliaddr);
    // Receive & Response 
    do {
        value v;
        n = recvfrom(sockfd,&v,sizeof(value),0,(struct sockaddr *)&cliaddr,&len);
        if (n < 0) error("recvfrom");
        struct header h = v.header;
        //printf("(%d,%s)\n", inst, v.value);
        values[index][inst] = h.key;  
        int seq = htonl(h.key >> 4);
        if (index == 0) {
            put(inst, v.value, strlen(v.value));
            n = sendto(sockfd,&seq,sizeof(seq), 0, (struct sockaddr *)&cliaddr, len);
            if (n < 0) error("sendto");
        }
        inst++;
        start_receiving = 1;
    }
    while (inst < MAX_SERVER);
    pthread_exit(NULL);
    return NULL;
}

void *evalFunc(void *args)
{
    int learn[MAX_SERVER];
    while (start_receiving == 0) {sleep(1);}
    struct timespec tstart={0,0}, tend={0,0}, res;
    clock_gettime(CLOCK_REALTIME, &tstart);
    int j, i = 0, k = 0;
    int decided_counter = 0;
    int undecided_counter = 0;
    FILE *out;
    char filename[20];
    char *tname;
    tname  = (char *)args;
    sprintf(filename, "/tmp/%s", tname);
    out = fopen(filename ,"w");
    if (out == NULL) {
        perror("Error opening file");
        exit(1);
    }
    int rc;
    struct timespec ts;
    while (cont && i < MAX_SERVER) { 
        rc = pthread_mutex_lock(&mutex);
        if (rc != 0) error("mutex lock");
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 1000000; // WAIT_TIME_SECONDS;
        int an_instance[4];
        for (j = 0; j < 4; j++) {
            if (values[j][i] != 0)  {
                an_instance[j] = values[j][i];
                if (printarray) fprintf(out, "%.8d\t", values[j][i]);
            } 
            else  {
                while (values[j][i] == 0)  {
                    rc = pthread_cond_timedwait(&cond, &mutex, &ts);
                    if (rc == ETIMEDOUT) {
                        an_instance[j] = -1;
                        if (printarray) fprintf(out, "%.8d\t");
                        break;
                    } else { an_instance[j] = values[j][i]; }
                }
            }
        }
        int selected = findMajorityElement(an_instance, 4); 
        if (selected != -1) decided_counter++;
        else undecided_counter++;
        if (printarray) fprintf(out, "%.8d\n", selected);
       
        //printf("inst:%d chosen:%8d\n", i, selected);
        learn[k++] = selected;
        if (printarray) fprintf(out, "%d\n", selected);
        // aggregate latency
        if ((i%100000) == 0) {
            clock_gettime(CLOCK_REALTIME, &tend);
            uint64_t res = timediff(tstart, tend);
            double duration =  res*1.0e-9;
            //printf("Duration: %.6f\n", duration);
            //printf("pps: %0.f\t", ((double) (undecided_counter + decided_counter) / duration));
            //printf("indecision: %.5f\n",(double)undecided_counter / (undecided_counter + decided_counter));
            fflush(out);
        }
        printf("%s\n", get(i));
        // Next instance
        i++;
        pthread_mutex_unlock(&mutex);
    }

    int thid;
    for(thid = 0; thid < 4; thid++) {
       pthread_cancel(tid[thid]);
    }
    fclose(out);
    //printf("End Eval thread. instance:%d\n", i);
}


int findMajorityElement(int* arr, int size) {
    int count = 0, i, majorityElement;
    for (i = 0; i < size; i++) {
        if (count == 0)
            majorityElement = arr[i];
        if (arr[i] == majorityElement)
            count++;
        else
            count--;
    }
    count = 0;
    for (i = 0; i < size; i++)
        if (arr[i] == majorityElement)
            count++;
    if (count > size/2)
        return majorityElement;
    return -1;
}

int main(int argc, char**argv)
{
    init_kv();
    int i, err;
    int count = 0;
    int *ptr[4];
    pthread_t eval_th; // thread to check majority of value 
    pthread_t recovery_thread;
    if (argc != 6) { printf("Usage: %s eth0 eth1 eth2 eth3 output.txt\n", argv[0]); exit(1);}

    for(i = 1; i < argc-1; i++) {
        struct thread_args *targ = malloc(sizeof(struct thread_args));
        targ->index = i-1;
        targ->itf = argv[i];
        err = pthread_create(&tid[count++], NULL, recvFunc, targ);
        if (err != 0) {
            perror("Thread create Error");
            exit(1);
        }
    }

    err = pthread_create(&eval_th, NULL, evalFunc, argv[argc-1]);
    if (err != 0) {
        perror("Thread create Error");
        exit(1);
    }

    for(i = 0; i < count; i++) {
       pthread_join(tid[i], (void**)&(ptr[i])); 
    }
    sleep(10);
    pthread_cancel(eval_th);
    cont = 0;
    pthread_join(eval_th, NULL);
}

/*
int send_multicast_socket(char* group, int port, value v) {

    struct sockaddr_in server;
    int n;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("create socket");

    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) error("fcntl error");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(group);
    server.sin_port = htons(port);
    unsigned int length = sizeof(struct sockaddr_in);

    fd_set write_fd_set;
    FD_ZERO(&write_fd_set);
    FD_SET(sock, &write_fd_set);
    

    while(1) {
        if (select(FD_SETSIZE, NULL, &write_fd_set, NULL, NULL) < 0)
            error("select");
      
        if (FD_ISSET(sock, &write_fd_set)) {
            // Run simple Paxos
            struct header h;
            h = v.header;
            h.msg_type = PREPARE;
            h.buffer_size = VALUE_SIZE;
            int msize = sizeof(struct header) + h.buffer_size;
            n = sendto(sock, &v, msize, 0, 
                        (struct sockaddr *)&server, length);
            if (n < 0) error("sendto");
        }
    }
}
*/
