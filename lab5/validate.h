#include <regex.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "args.h"

#ifndef VALIDATE_H
#define VALIDATE_H

int match(const char *string, char *pattern)
{
    int        status;
    regex_t    re;

    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
      return(0);
    }
    status = regexec(&re, string, (size_t) 0, NULL, 0);
    regfree(&re);
    if (status != 0) {
   	  return(0);
    }
    return(1);
}
//==============================================================================
//Get data from socket and validate it
void  server_validate(int fd)
{
	char  recvBuff[1024];
	char  sendBuff[1024];
	char * currentStr;

	recv(fd, recvBuff, strlen(recvBuff),0);
	
	currentStr 		= malloc(20);

	int i;

	printf("recieve data");

	for(i=0;i<strlen(recvBuff);i++)
	{
		while(recvBuff[i]!="\n")
		{
			strcat(currentStr,recvBuff[i]);
		}
		if(match(currentStr, "^[a-zA-Z0-9]+@[a-zA-Z0-9]+[.][a-zA-Z0-9]+$"))
		{
			strcat(sendBuff,currentStr);
			strcat(sendBuff,"\n");
		}
		i++;

	}
	strcat(sendBuff,"\0");
	printf("send results");
	send(fd, sendBuff, strlen(sendBuff), 0);
	
	free(currentStr);
}
//==============================================================================
//Read strings from file and send to server
void client_validate(struct Args * args)
{
	printf("client_validate");
	int sockfd = args->fd, n = 0;
    char recvBuff[1024];
    char sendBuff[1024];
    struct sockaddr_in serv_addr; 
    FILE * validFile;
	FILE * sourceFile;
	FILE * log;
	char * validFilename;
	char * currentStr;
	char * filename;
	
	currentStr 	  = malloc(20);
	filename 	  = malloc(30);
	validFilename = malloc(30);
	
	memset(sendBuff, '0',sizeof(recvBuff));
	memcpy(filename, args->args, strlen(args->args));
	memcpy(validFilename, filename, strlen(filename));
	strcat(validFilename, ".valid");

	sourceFile  = fopen(filename, "r");
	
	
	while(fgets(currentStr, 20, sourceFile))
	{
		int i;
		for(i=0;i<strlen(currentStr);i++)
		{
			if (currentStr[i]=='\n')
			{
				currentStr[i]='\0';
			}
		}

		strcat(sendBuff, currentStr);
	}

	fclose(sourceFile);
	
	free(currentStr);
	log = fopen("client.log", "w");
    validFile 	= fopen(validFilename, "w");
    write(log, "log opened", 10);
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
    	fputs(log, "Error : Connect Failed");
        printf("\n Error : Connect Failed \n");
        return 1;
    } 

    if( send(sockfd , sendBuff , strlen(sendBuff) , 0) < 0)
    {
    	fputs(log, "Error : Send Failed");
        printf("Send failed");
        return 1;
    }



    if( recv(sockfd, recvBuff, sizeof(recvBuff),0) < 0)
    {
    	fputs(log, "Error : Recieve Failed");
    	printf("Recieve failed");
        return 1;
    }
    else
    {

    	printf("write to file");
    	write(validFile,recvBuff,strlen(recvBuff));
    	fclose(validFile);
    }
    fputs(log, "before close log");
    fclose(log);
    pthread_exit(NULL);
}
#endif //VALIDATE_H