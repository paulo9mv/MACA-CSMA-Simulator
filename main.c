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
    int i = 0;
	int newsockfd;
	int portno;
	int clilen;
	char buffer[801];
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t threads[100];

    int sockfd;

    //Inicializa o socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd < 0) {
		printf("Socket error!\n");
		exit(1);
	}



	memset((char*) &serv_addr, 0, sizeof(serv_addr));

	portno = 10;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Socket error!\n");
		exit(1);
	}

	listen(sockfd, 100);
	clilen = sizeof(cli_addr);

    while(newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, (unsigned int*) &clilen)) {
		if(pthread_create(&(threads[i++]), NULL, new_conection, (void*) &newsockfd) < 0) {
			printf("Thread creation error!\n");
			exit(1);
		}
	}

	if(newsockfd < 0) {
		printf("Newsocket error!\n");
		exit(1);
	}
}
int main(){
    int sock;
    pthread_t thread, thread2;

    //Inicializa o socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0) {
		printf("Socket error\n");
		exit(1);
	}

    pthread_create(&thread, NULL, receive, NULL);
    pthread_create(&thread2, NULL, sendThread, NULL);

    pthread_join(thread2, NULL);
    pthread_join(thread, NULL);

    char data[800];
    payload(data, 920);

}
