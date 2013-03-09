#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "args.h"
#include <signal.h>

#ifndef SERVER_H
#define SERVER_H

int                s_listenfd; //listener descriptor
int                connfd[10];// connection descriptor

void s_term_handler(int sig)
{
  int i;
  printf("term_handler\n");
  if(sig==SIGINT||sig==SIGABRT)
  {
    for(i=0;i<10;i++)
    {
      close (connfd[i]);
    }
    shutdown(s_listenfd,2);
    close(s_listenfd);
    exit(0);
  }
}

//==============================================================================
//Get data from socket and validate it
void  server_validate(struct ThreadArgs * args)
{
  char  recvBuff[1025];
  char  sendBuff[1024];
  char  sendMess[1020];
  char  messSize[4];
  char  currentStr[20];
  int   i;
  int   id;
  struct sockaddr_in serv_addr; 
  int   work_listenfd;
  int optval = 1;

  id = args->currentId;

  work_listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(sendBuff,   '\0', sizeof(sendBuff)); 
  memset(sendMess,   '\0', sizeof(sendMess)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(5001+id); 

  setsockopt(work_listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

  bind(work_listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

  listen(work_listenfd, 10);

  printf("listening port %d\n", 5001+id );

  connfd[id] = accept(work_listenfd, (struct sockaddr*)NULL, NULL);

  memset(recvBuff, '0',sizeof(recvBuff));
  memset(currentStr, '\0',sizeof(currentStr));

  while(!read(connfd[id], &recvBuff, strlen(recvBuff))){}
  
  for(i=0;i<strlen(recvBuff)&&i<20;i++)
  {
    while(recvBuff[i]!='\n'&&i<strlen(recvBuff))
    {
      strncat(currentStr,&recvBuff[i],1);
      i++;
    }
    if(match(currentStr, "^[a-zA-Z0-9]+@[a-zA-Z0-9]+[.][a-zA-Z0-9]+$"))
    {
      strcat(sendMess,currentStr);
      strcat(sendMess,"\n");
    }
    memset(currentStr, '\0',sizeof(currentStr));
  }

  sprintf(messSize,"%d",strlen(sendMess));

  strcat(sendBuff,messSize);
  strcat(sendBuff, "\n");
  strcat(sendBuff,sendMess);

  sleep(10);

  write(connfd[id], sendBuff, strlen(sendBuff));

  args->pull[id] = 0;

  close(connfd[id]);
  shutdown(work_listenfd,2);
  close(work_listenfd);

  pthread_exit(NULL);
}

//==============================================================================
//Get connect from client and create worker
void server()	
{
  int optval = 1;
  int listen_conn;

  struct sockaddr_in serv_addr; 

  int                tPull[10]={0};//threads pull
  pthread_t          threads[10];//threads array

  s_listenfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port        = htons(5000); 

  setsockopt(s_listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

  bind(s_listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

  listen(s_listenfd, 10); 

  struct ThreadArgs args;
  args.currentId = 0;

  while(1)
  {
    listen_conn = accept(s_listenfd, (struct sockaddr*)NULL, NULL);

    int i=0;

    for(i=0;i<10;i++)
    {
      if(tPull[i]==0)
      {
        char work_port[5];

        args.pull = tPull;
        args.currentId = i;

        sprintf(work_port, "%d", args.currentId+5001);

        tPull[i] = 1;
   
        write(listen_conn, &work_port, 5);

        printf("created new thread with id %d\n", args.currentId);

        pthread_create( &threads[i], NULL, &server_validate, (void*) &args);

        close(listen_conn);

        break;
      }
    }
  }
}


#endif //SERVER_H