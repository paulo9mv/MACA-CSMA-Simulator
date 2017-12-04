#include<stdio.h>
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

#define PORT 3900
#define BUFFERSIZE 20

typedef struct data{
    int station;
    int port;
    int newConnection;
}Data;

void *payload(char payload[],int station){
    int i;
    for(i = 0; i < BUFFERSIZE; i++, station++){
        if(station < 10)
            payload[i] = station + '0';
        else if(station < 100){
            payload[i] = (station / 10) + '0';
            if(i + 1 < BUFFERSIZE)
                payload[++i] = (station % 10) + '0';
        }else{
            payload[i] = (station / 100) + '0';
            if(i + 1 < BUFFERSIZE)
                payload[++i] = (station/10)%10 + '0';
            if(i + 1 < BUFFERSIZE)
                payload[++i] = station % 10 + '0';
        }
    }
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
    struct sockaddr_in receiveAddr;
    Data d = *(Data *)a;
    int port = d.port - 1;
    if(port < 3100)
        port = 3109;
    int station = d.station;
    char buffer[305];

    if(sock < 0){
        printf("Erro no socket\n");
        exit(1);
    }

    pthread_t thread;
    memset(&receiveAddr, 0, sizeof(receiveAddr));

    receiveAddr.sin_family = AF_INET;
    receiveAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    receiveAddr.sin_port = htons(port);

    bind(sock, (struct sockaddr*)&receiveAddr, sizeof(receiveAddr));
    listen(sock, 5);

    newConnection = accept(sock, (struct sockaddr*)NULL, NULL);
    if(newConnection < 0){
        printf("Connection failed\n");
        exit(1);
    }
    d.newConnection = newConnection;
    pthread_create(&thread, NULL, receiveIndividual, (void *)&d);
    pthread_join(thread, NULL);

    printf("R Connect %d\n", port);




}
void *sender(void *a){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in receiveAddr;
    int read_size;

    Data d = *(Data *)a;
    int port = d.port;
    int station = d.station;

    char buffer[BUFFERSIZE];
    payload(buffer, station);

    if(sock < 0){
        printf("Erro no socket\n");
        exit(1);
    }

    memset(&receiveAddr, 0, sizeof(receiveAddr));

    receiveAddr.sin_family = AF_INET;
    receiveAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    receiveAddr.sin_port = htons(port);

    if(connect(sock, (struct sockaddr*)&receiveAddr, sizeof(receiveAddr)) < 0){
        printf("Connect fail %d\n", port);
        exit(1);
    }
    printf("S Connect %d\n", port);

    write(sock, buffer, sizeof(buffer) + 50);
}

int main(){
    int N = 10, i;
    pthread_t thread[2];
    int port1 = 2888, port2 = 3788;
    int j ;

    for(i = 0; i < N; i++){
        if(fork() == 0){
            Data t;
            t.station = i;
            t.port = i + 3100;


            pthread_create(&thread[1], NULL, receiver, (void *)&t);
            pthread_create(&thread[2], NULL, sender, (void *)&t);

            pthread_join(thread[1], NULL);
            pthread_join(thread[2], NULL);
            exit(0);
    }
}

    return 0;
}
