#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>


#define PORT 8888
#define SIZE 1470
#define MAX_NUM 1000000 + 5
#define BILLION 1000000000L

pthread_t tid[4]; // this is thread identifier

int values[4][MAX_NUM];  // value queues
uint64_t ltc[MAX_NUM];
bool start = false;
int counter = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int idx = 0;

pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;


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
    char mesg[SIZE];
    char *itf;
    itf = (char*)arg; 
    pthread_mutex_lock(&lock);
    int index = idx++;
    pthread_mutex_unlock(&lock);
    pthread_t self_id;
    self_id = pthread_self();
    //printf("index:%d interface %s\n", index, itf);
    char last_msg[9];
    char sec[10];
    char nsec[9];
    int last_id = 0;
    int inst = 0;
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
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
    bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    
    
    struct timespec end;
 
    for (;;)
    {
        len = sizeof(cliaddr);
        n = recvfrom(sockfd,mesg,SIZE,0,(struct sockaddr *)&cliaddr,&len);

        clock_gettime(CLOCK_REALTIME, &end);

        strncpy(last_msg, mesg, 8);
        strncpy(sec, mesg+8, 10);
        strncpy(nsec, mesg+19, 9);

        struct timespec start = {atoi(sec), atoi(nsec)};
        //printf("%d.%d\n", start.tv_sec, start.tv_nsec);
        uint64_t diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
        //printf("latency = %llu us\n", (long long unsigned int) diff / 1000);
        ltc[inst] = diff;
        
        
        pthread_mutex_lock(&lock);
        if(inst > counter) 
        {
            counter = inst;
        }
        pthread_mutex_unlock(&lock);
       
        //if (index == 0) counter++;
        //printf("index: %d counter:%d\n", index, counter);
        last_msg[9] = '\0'; // place the null terminator
        last_id = atoi(last_msg);
        values[index][inst] = last_id;  
        inst++;
    }
    pthread_exit(&last_id);
    return NULL;
}

void *evalFunc(void *args)
{
    int learn[MAX_NUM];
    while (counter == 0) {sleep(1);}
    printf("start eval\n");
    struct timespec tstart={0,0}, tend={0,0}, res;
    clock_gettime(CLOCK_REALTIME, &tstart);
    int j, i = 0, k = 0;
    int decided_counter = 0;
    int undecided_counter = 0;

    uint64_t avg_ltc = 0.0; 

    FILE *out;
    char filename[20];
    char *tname;
    tname  = (char *)args;
    sprintf(filename, "/tmp/%s", tname);
    printf("filename:%s\n", filename);
    out = fopen(filename ,"w");

    if (out == NULL) {
        perror("Error opening file");
        exit(1);
    }

    int rc;
    struct timespec ts;
    struct timeval  tp;
    
    rc = pthread_mutex_lock(&mutex);
    while (true) { 
        rc =  gettimeofday(&tp, NULL);
        /* Convert from timeval to timespec */
        ts.tv_sec  = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += 1; // WAIT_TIME_SECONDS;
        int an_instance[4];
        for (j = 0; j < 4; j++) {
            if (values[j][i] != 0)  {
                an_instance[j] = values[j][i];
                //printf("%.8d  ", values[j][i]);
            } 
            else  {
                while (values[j][i] == 0)  {
                    //printf("Thread blocked: interface %d\n", j);
                    rc = pthread_cond_timedwait(&cond, &mutex, &ts);
                    if (rc == ETIMEDOUT) {
                        //printf("Wait timed out!\n");
                        rc = pthread_mutex_unlock(&mutex);
                        an_instance[j] = -2;
                        //printf("%8s ", "");
                        break;
                    }
                    else     
                        an_instance[j] = values[j][i];
                }
            }
        }
        //printf("\n");
        int selected = findMajorityElement(an_instance, 4); 
        if (selected != -1) decided_counter++;
        else undecided_counter++;
       
        //printf("inst:%d chosen:%8d\n", i, selected);
        learn[k++] = selected;
        fprintf(out, "%d\n", selected);
        // aggregate latency
        avg_ltc += ltc[i];
        if ((i%100000) == 0) {
            clock_gettime(CLOCK_REALTIME, &tend);
            uint64_t res = BILLION * (tend.tv_sec - tstart.tv_sec) + tend.tv_nsec - tstart.tv_nsec;
            double duration =  res*1.0e-9;
            printf("Duration: %.6f\n", duration);
            printf("Packet/second: %0.f\n", ((double) counter / duration));
            printf("Ratio of undicided req: %.5f\n",
                                    (double)undecided_counter / counter);
            printf("avg_latency: %.2f us\n", 1.0e-3*avg_ltc/((double)i+1));
            fflush(out);
        }
        // Next instance
        i++;
    }

    
    fclose(out);
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
   /* 
     if (pthread_mutex_init(&lock, NULL) != 0)
    {
        perror("mutex init failed\n");
        return 1;
    }
    */

    for(i = 1; i < argc-1; i++) {
        err = pthread_create(&tid[count++], NULL, recvFunc, argv[i]);
        if (err != 0) {
            perror("Thread create Error");
            exit(1);
        }
    }

    err = pthread_create(&tid[count++], NULL, evalFunc, argv[argc-1]);
    if (err != 0) {
        perror("Thread create Error");
        exit(1);
    }

    for(i = 0; i < count; i++) {
       pthread_join(tid[i], (void**)&(ptr[i])); 
    }
}
