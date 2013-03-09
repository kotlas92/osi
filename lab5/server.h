#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "args.h"

#ifndef SERVER_H
#define SERVER_H

//==============================================================================
//Get data from socket and validate it
void  server_validate(struct ThreadArgs * args)
{
  char  recvBuff[1025];
  char  sendBuff[1024];
  char  currentStr[20];
  int i;
  int listenfd;
  int connfd;
  struct sockaddr_in serv_addr; 

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(sendBuff,   '\0', sizeof(sendBuff)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(5001+args->currentId); 

  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

  listen(listenfd, 10);

  connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

  memset(recvBuff, '0',sizeof(recvBuff));
  memset(currentStr, '\0',sizeof(currentStr));

  while(!read(connfd, &recvBuff, strlen(recvBuff))){}
  
  for(i=0;i<strlen(recvBuff)&&i<20;i++)
  {
    while(recvBuff[i]!='\n'&&i<strlen(recvBuff))
    {
      strncat(currentStr,&recvBuff[i],1);
      i++;
    }
    if(match(currentStr, "^[a-zA-Z0-9]+@[a-zA-Z0-9]+[.][a-zA-Z0-9]+$"))
    {
      strcat(sendBuff,currentStr);
      strcat(sendBuff,"\n");
    }
    memset(currentStr, '\0',sizeof(currentStr));
  }
  
  write(connfd, sendBuff, strlen(sendBuff));

  args->pull[args->currentId] = 0;

  close(connfd);
  close(listenfd);

  pthread_exit(NULL);
}

//==============================================================================
//Get connect from client and create worker
void server()	
{
  int                listenfd; //listener descriptor
  int                connfd;// connection descriptor
  struct sockaddr_in serv_addr; 

  int                tPull[10]={0};//threads pull
  pthread_t          threads[10];//threads array

  

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port        = htons(5000); 

  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

  listen(listenfd, 10); 

  struct ThreadArgs args;

  while(1)
  {
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    int i=0;

    for(i=0;i<10;i++)
    {
      if(!tPull[i])
      {
        args.pull = tPull;
        args.currentId = i;

        tPull[i] = 1;
   
        write(connfd, &"5001\0", 5);

        printf("created new thread with id %d\n", args.currentId);

        pthread_create( &threads[i], NULL, &server_validate, (void*) &args);

        break;
      }
    }
  }
}


#endif //SERVER_H