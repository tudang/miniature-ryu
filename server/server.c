#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define PORT 8888
#define SIZE 1470
#define MAX_NUM 1000000 
pthread_t tid[4]; // this is thread identifier

int values[4][MAX_NUM];  // value queues
bool start = false;
int counter = 0;
pthread_mutex_t lock;
int idx = 0;
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
    printf("\nHello from thread %u\n",self_id);
    printf("index:%d interface %s\n", index, itf);
    char last_msg[9];
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
    
    for (;;)
    {
        len = sizeof(cliaddr);
        n = recvfrom(sockfd,mesg,SIZE,0,(struct sockaddr *)&cliaddr,&len);
        //sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
        //printf("[%s]%.8s\n", itf, mesg);
        strncpy(last_msg, mesg, 8);
        last_msg[9] = '\0'; // place the null terminator
        last_id = atoi(last_msg);
        values[index][inst++] = last_id;  
        if (counter < inst) counter = inst;
        if (last_id >= 999900) break; 
    }
        pthread_exit(&last_id);
    return NULL;
}

void *evalFunc(void *args)
{
    int learn[MAX_NUM];
    while (counter == 0) {sleep(1);}
    
    int j, i = 0, k = 0;
    while (i < counter) {
        int an_instance[4];
        for (j = 0; j < 4; j++) {
            if (values[j][i] != 0) {
                //printf("%.8d  ", values[j][i]);
                an_instance[j] = values[j][i];
            } else {
                an_instance[j] = -1; 
                //printf("%8s ", "");
            }
        }
        int selected = findMajorityElement(an_instance, 4); 
        printf("%8d\n", selected);
        learn[k++] = selected;
        i++;
        if (i == counter) sleep(1);
    }
    printf("End eval: counter=%d\n", counter);
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
    
     if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }


    for(i = 1; i < argc; i++) {
        err = pthread_create(&tid[count++], NULL, recvFunc, argv[i]);
        if (err != 0) {
            perror("Thread create Error");
            exit(1);
        }
    }

    err = pthread_create(&tid[count++], NULL, evalFunc, (void *)NULL);
    if (err != 0) {
        perror("Thread create Error");
        exit(1);
    }

    for(i = 0; i < count; i++) {
       pthread_join(tid[i], (void**)&(ptr[i])); 
    }

    for(i = 0; i < count; i++) {
       printf("Last received msg: [%d]\n", *ptr[i]);
    }
}
