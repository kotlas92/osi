#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "validate.h"
#include "args.h"

#ifndef CLIENT_H
#define CLIENT_H

void client(int argc, char ** argv)
{
	int    i;//counter to creating threads
	int    filesCount;
	int    sockfd;
	pthread_t thread_id[10];
	int connfd = 0;
    struct sockaddr_in serv_addr; 

	filesCount = (argc-1)/2;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    struct Args args;
    args.fd = sockfd;

    printf("filesCount %d\n", filesCount);

	//open threads
	for(i=1;i <= filesCount;i++)
	{
		printf("to strcmp %s\n", argv[i*2]);
		printf("strcmp: %d\n", strcmp(argv[i*2],"-f") );
		if (!strcmp(argv[i*2],"-f"))
		{
			printf("thread created");
			strcpy(args.args,argv[i*2+1]);
			pthread_create( &thread_id[i], NULL, &client_validate, (void*) &args);
		}
	}

	printf("threads created");

	//join threads
	for(i=1;i <= filesCount;i++)
	{
		printf("tmp");
		if (!strcmp(argv[i*2],"-f"))
		{
			pthread_join(thread_id[i], NULL);
			printf("thread joined");
		}
	}

	printf("Succesfully validate\n");
}
#endif //CLIENT_H