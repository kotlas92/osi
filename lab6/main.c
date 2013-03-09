#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "validate.h"
#include "server.h"
#include "client.h"

int main(int argc, char ** argv)
{
	signal(SIGINT, s_term_handler);
	signal(SIGINT, c_term_handler);
	if((argc < 2)||((argc == 2) && strcmp(argv[1],"-s"))||((argc > 2)&&((argc < 4)||strcmp(argv[1],"-c")||strcmp(argv[2],"-f"))))
	{
		printf("Error. Use format: \n    -s             :to server-mode\n    -c -f filename :to client-mode\n");
		return 1;
	}

	if(argc == 2)
	{
		server();
	}
	else
	{
		client(argc,argv);
	}
	return 0;
}