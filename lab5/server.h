#include <regex.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifndef SERVER_H
#define SERVER_H
void server()	
{
  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr; 

  char sendBuff[1025];
  time_t ticks; 


  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(sendBuff, '0', sizeof(sendBuff)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(5000); 

  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

  listen(listenfd, 10); 

  while(1)
  {
  	pthread_t thread_id;

    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
    printf("create new thread");
    pthread_create( &thread_id, NULL, &server_validate, (void*) connfd);
  }
}

#endif //SERVER_H