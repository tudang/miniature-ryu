#include "value.h"

pthread_t tid[4]; // this is thread identifier

int values[4][MAX_SERVER];  // value queues
int start_receiving = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int idx = 0;

pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;

int cont = 1; // share variable between main & eval thread

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
    char mesg[BUF_SIZE];
    char *itf;
    itf = (char*)arg; 
    pthread_mutex_lock(&lock);
    int index = idx++;
    pthread_mutex_unlock(&lock);
    pthread_t self_id;
    self_id = pthread_self();
    //printf("index:%d interface %s\n", index, itf);
    value v;
    int inst = 0;
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
    if (inet_ntop(AF_INET, &sin, itf_addr, sizeof(itf_addr)))
        {
        //printf("%s\n", itf_addr);
        }
    else
        perror("inet_ntop");
    
    
    struct ip_mreq mreq;

    mreq.imr_multiaddr.s_addr = inet_addr(GROUP);
    mreq.imr_interface.s_addr = inet_addr(itf_addr);

    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        error("setsockopt mreq");
        exit(1);
    }
    
    if (bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        error("bind");
        exit(1);
    }
    len = sizeof(cliaddr);
    // Receive & Response 
    do {
        n = recvfrom(sockfd,mesg,BUF_SIZE,0,(struct sockaddr *)&cliaddr,&len);
        if (n < 0) error("recvfrom");
        deserialize_value(mesg, &v);
        values[index][inst++] = v.client_id*1000000 + v.sequence;  
        int seq = htonl(v.sequence);
        n = sendto(sockfd,&seq,sizeof(seq), 0, (struct sockaddr *)&cliaddr, len);
        if (n < 0) error("sendto");
        start_receiving = 1;
    }
    while (inst < MAX_SERVER);
    //printf("index:%d interface %s instance: %d\n", index, itf, inst);
    pthread_exit(NULL);
    return NULL;
}

void *evalFunc(void *args)
{
    int learn[MAX_SERVER];
    while (start_receiving == 0) {sleep(1);}
    //printf("start eval\n");
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
    //printf("filename:%s\n", filename);
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
        ts.tv_sec += 1; // WAIT_TIME_SECONDS;
        int an_instance[4];
        for (j = 0; j < 4; j++) {
            if (values[j][i] != 0)  {
                an_instance[j] = values[j][i];
                printf("%.8d\t", values[j][i]);
            } 
            else  {
                while (values[j][i] == 0)  {
                    rc = pthread_cond_timedwait(&cond, &mutex, &ts);
                    if (rc == ETIMEDOUT) {
                        //printf("Thread blocked: interface %d\n", j);
                        //printf("Wait timed out!\n");
                        an_instance[j] = -1;
                        printf("%.8d\t");
                        break;
                    }
                    else     
                    {
                        an_instance[j] = values[j][i];
                        //printf("Wait not timed out!\n");
                    }
                }
            }
        }
        int selected = findMajorityElement(an_instance, 4); 
        if (selected != -1) decided_counter++;
        else undecided_counter++;
        printf("%.8d\n", selected);
       
        //printf("inst:%d chosen:%8d\n", i, selected);
        learn[k++] = selected;
        fprintf(out, "%d\n", selected);
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
    int i, err;
    int count = 0;
    int *ptr[4];
    pthread_t eval_th; // thread to check majority of value 
   /* 
     if (pthread_mutex_init(&lock, NULL) != 0)
    {
        perror("mutex init failed\n");
        return 1;
    }
    */
    if (argc != 6) { printf("Usage: %s eth1 eth2 eth3 eth4 output.txt\n", argv[0]); exit(1);}

    for(i = 1; i < argc-1; i++) {
        err = pthread_create(&tid[count++], NULL, recvFunc, argv[i]);
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

    sleep(20);
    
    cont = 0;
 
    pthread_join(eval_th, NULL);

    //printf("Main end\n");
    //pthread_cancel(eval_th);
}
