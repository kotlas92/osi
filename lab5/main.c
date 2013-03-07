#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "validate.h"

int main(int argc, char ** argv)
{
	if((argc < 2)||((argc == 2) && strcmp(argv[1],"-s"))||((argc > 2)&&((argc < 4)||strcmp(argv[1],"-c")||strcmp(argv[2],"-f"))))
	{
		printf("Error. Use format: \n    -s             :to server-mode\n    -c -f filename :to client-mode\n");
		return 1;
	}

	if(argc == 2)
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
	        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

	        snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
	        write(connfd, sendBuff, strlen(sendBuff)); 

	        close(connfd);
	        sleep(1);
	     }
	}
	else
	{
		int sockfd = 0, n = 0;
	    char recvBuff[1024];
	    struct sockaddr_in serv_addr; 

	    // if(argc != 2)
	    // {
	    //     printf("\n Usage: %s <ip of server> \n",argv[0]);
	    //     return 1;
	    // } 

	    memset(recvBuff, '0',sizeof(recvBuff));
	    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    {
	        printf("\n Error : Could not create socket \n");
	        return 1;
	    } 

	    memset(&serv_addr, '0', sizeof(serv_addr)); 

	    serv_addr.sin_family = AF_INET;
	    serv_addr.sin_port = htons(5000); 

	    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
	    {
	        printf("\n inet_pton error occured\n");
	        return 1;
	    } 

	    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	    {
	       printf("\n Error : Connect Failed \n");
	       return 1;
	    } 

	    while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
	    {
	        recvBuff[n] = 0;
	        if(fputs(recvBuff, stdout) == EOF)
	        {
	            printf("\n Error : Fputs error\n");
	        }
	    } 

	    if(n < 0)
	    {
	        printf("\n Read error \n");
	    } 

	    return 0;

		// int    i;//counter to creating threads
		// int    filesCount;
		// pthread_t thread_id[10];

		// filesCount = (argc-1)/2;

		// //open threads
		// for(i=1;i <= filesCount;i+=2)
		// {
		// 	if (!strcmp(argv[i],"-f"))
		// 	{
		// 		pthread_create( &thread_id[i], NULL, &validate, (void*) argv[i+1]);
		// 	}
		// }

		// //join threads
		// for(i=1;i <= filesCount;i+=2)
		// {
		// 	if (!strcmp(argv[i],"-f"))
		// 	{
		// 		pthread_join(thread_id[i], NULL);
		// 	}
		// }

		// printf("Succesfully validate\n");
	}
	return 0;
}