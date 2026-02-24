#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/udp.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<string.h> /* memset */
#include<unistd.h> /* close */
#include<time.h>
#include<pthread.h>
#include<err.h>
#include<sys/mman.h>
#include<net/ethernet.h>

#define PORT 3990
#define SEED 999
#define N 5
#define BUFSIZE 200
#define BUF_SIZ 216

#define DEFAULT_PORT 3000
#define DEFAULT_LISTEN 3400
#define SOCKET_ERROR -1

#define MAX_PIDS 101

typedef struct data{
    int station;
    int port;
    int newConnection;
}Data;

volatile pid_t *pids;

void payload(char payload[],int station){
    int i;
    const int hdrlen = (int)sizeof(struct ether_header);

    char macsrc[50];
    char macdest[50];

    struct ether_header *eh = (struct ether_header *) payload;

    /* Construct the Ethernet header */
    memset(payload, 0, BUF_SIZ);
    /* Ethernet header */       
    memcpy(eh->ether_shost, macsrc, ETH_ALEN);
    memcpy(eh->ether_dhost, macdest, ETH_ALEN);
    /* Ethertype field */
    eh->ether_type = htons(ETH_P_IP);

    for(i = hdrlen; i < BUFSIZE + hdrlen; i++, station++){
        if(station < 10)
            payload[i] = station + '0';
        else if(station < 100){
            payload[i] = (station / 10) + '0';
            if(i + 1 < BUFSIZE + hdrlen)
                payload[++i] = (station % 10) + '0';
        }else{
            payload[i] = (station / 100) + '0';
            if(i + 1 < BUFSIZE + hdrlen)
                payload[++i] = (station/10)%10 + '0';
            if(i + 1 < BUFSIZE + hdrlen)
                payload[++i] = station % 10 + '0';
        }
    }
}
int neigh(int a, int neigh[]){
    int i;
    for(i = 0; i < 2; i++){
        if(neigh[i] == a)
            return 1;
    }
    return 0;
}
void *receiveIndividual(void *a){
    Data d = *(Data *)a;
    int socket = d.newConnection;
    int station = d.station;
    char buffer[300];
    int read_size = recv(socket, buffer, 100, 0);

    if(read_size > 0)
        printf("Estação %d recebeu %s\n",station ,buffer);
}
void *receiver(void *a){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int newConnection;
    int read_size;
    int i = 0;
    struct sockaddr_in receiveAddr;
    Data d = *(Data *)a;
    int port = d.port;
    int station = d.station;
    char buffer[305];

    if(sock < 0){
        printf("Erro no socket\n");
        exit(1);
    }

    pthread_t thread[10];
    memset(&receiveAddr, 0, sizeof(receiveAddr));

    receiveAddr.sin_family = AF_INET;
    receiveAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    receiveAddr.sin_port = htons(port);

    bind(sock, (struct sockaddr*)&receiveAddr, sizeof(receiveAddr));
    listen(sock, 10);

    while(i < N){
        newConnection = accept(sock, (struct sockaddr*)NULL, NULL);
        if(newConnection < 0){
            printf("Connection failed\n");
            exit(1);
        }
        d.newConnection = newConnection;
        pthread_create(&thread[i], NULL, receiveIndividual, (void *)&d);
        pthread_join(thread[i], NULL);
        i++;
    }
}


int freeAmbient(int station, int *neighbour){
    int i;

    for(i = 0; i < 2; i++)
        if(pids[neighbour[i]] == 1)
            return 0;

    //nao ha transmissao dos vizinhos, envio OK
    pids[station] = 1;

    return 1;
}
int treesholder(int a){
    int b = rand()%100;
    if(b <= a)
        return 0;
    return 1;
}

void *sender(void *a){
    int i;
    int sock[N];
    char buffer[BUFSIZE];
    struct sockaddr_in receiveAddr[N];

    for(i = 0; i < 2; i++){
        sock[i] = socket(AF_INET, SOCK_STREAM, 0);
        if(sock[i] < 0){
            printf("Socket error\n");
            exit(1);
        }
    }


    Data d = *(Data *)a;
    int port[2];
    int station = d.station;
    int thresoulder = 0;

    int neighbours[2];

    if(station - 1 < 0){
        neighbours[0] = N - 1;
        port[0] = N - 1 + PORT;
    }
    else{
        neighbours[0] = station - 1;
        port[0] = station - 1 + PORT;
    }
    if(station + 1 >= N){
        neighbours[1] = 0;
        port[1] = PORT;
    }
    else{
        neighbours[1] = station + 1;
        port[1] = station + 1 + PORT;
    }

    for(i = 0; i< 2; i++){
        memset(&receiveAddr[i], 0, sizeof(receiveAddr));

        receiveAddr[i].sin_family = AF_INET;
        receiveAddr[i].sin_addr.s_addr = htonl(INADDR_ANY);
        receiveAddr[i].sin_port = htons(port[i]);
    }

    payload(buffer, station);

    while(freeAmbient(station, neighbours) == 0){
        usleep(1000 + (rand()%100000));
    }

    for(i = 0; i < 2; i++){
        //Faz uma conexao e envia para cada vizinho
        if(connect(sock[i], (struct sockaddr*)&receiveAddr[i], sizeof(receiveAddr)) < 0){
            printf("Connect fail %d\n", port[i]);
            exit(1);
        }

        while(thresoulder == 0){
            thresoulder = treesholder(10);
            if(thresoulder == 1)
            if(write(sock[i], buffer, sizeof(buffer)) == -1)
                printf("Erro\n");
        }

        thresoulder = 0;
        close(sock[i]);
    }

    //Libera dizendo que já liberou o canal
    pids[station] = 0;
}

int main(){
    int i, j;
    srand(SEED);
    // Map space for shared array
 pids = mmap(0, MAX_PIDS*sizeof(pid_t), PROT_READ|PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
 if (!pids) {
   perror("mmap failed");
   exit(1);
 }
 memset((void *)pids, 0, MAX_PIDS*sizeof(pid_t));

 for(i = 0; i < N; i++){
     pids[i] = 0;
 }

    int aleatorio = rand()%10;

    for(i = 0; i < N; i++){
        if(fork() == 0){
            pthread_t thread[2];
            Data t;
            t.station = i;
            t.port = i + PORT + aleatorio;

            sleep(1);

            pthread_create(&thread[0], NULL, receiver, (void *)&t);
            pthread_create(&thread[1], NULL, sender, (void *)&t);

            pthread_join(thread[0], NULL);
            pthread_join(thread[1], NULL);
            exit(0);
    }
}
    sleep(3);
    return 0;
}
