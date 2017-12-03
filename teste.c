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
void quebra(){
    printf("\n");
}
char *payload(char payload[],int station){
    int i;
    for(i = 0; i < 800; i++, station++){
        if(station < 10)
            payload[i] = station + '0';
        else if(station < 100){
            payload[i] = (station / 10) + '0';
            if(i + 1 < 800)
                payload[++i] = (station % 10) + '0';
        }else{
            payload[i] = (station / 100) + '0';
            if(i + 1 < 800)
                payload[++i] = (station/10)%10 + '0';
            if(i + 1 < 800)
                payload[++i] = station % 10 + '0';
        }
    }
    return payload;
}
void *new_conection(void *a){
    int newsock = *(int*)a;

    char buffer[801];

	memset(buffer, 0, sizeof(buffer));

	if(read(newsock, buffer, sizeof(buffer)) < 0) {
		printf("Read error\n");
		exit(1);
	}

	printf("Here is the message: %s", buffer);

	if(write(newsock, "Recebi a mensagem", 18) < 0) {
		printf("Send error\n");
		exit(1);
	}

	close(newsock);
}
void *receive(){
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    char sendBuff[1025];
    time_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0){
        printf("Socket error!\n");
        exit(1);
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);

    while(1)
    {
        printf("Aguardando conexão\n");
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

        ticks = time(NULL);
        snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        write(connfd, sendBuff, strlen(sendBuff));

        close(connfd);
        sleep(1);
     }
}
void *sendThread(){
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr;

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        exit(1);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        exit(1);
    }
    else{
        printf("OK\n");
    }

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       exit(1);
    }
    else{
        printf("CONNECTION SUCESSFULL!\n");
    }
    write(sockfd, )

    exit(1);
}
int main(){
    pthread_t thread, thread2;

    pid_t process;
    process = fork();

    if(process == 0){
        printf("Filho: %d\n", process);
        sleep(1);
        pthread_create(&thread, NULL, receive, NULL);
        pthread_join(thread, NULL);
    }
else{
    printf("Pai: %d\n", process);
    sleep(2);
    pthread_create(&thread2, NULL, sendThread, NULL);
    pthread_join(thread2, NULL);
}




    char data[800];
    payload(data, 920);

    return 0;

}
