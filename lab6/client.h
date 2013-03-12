#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef CLIENT_H
#define CLIENT_H

#define IP "127.0.0.1"

int                sockfd;// connection descriptor

void c_term_handler(int sig)
{
  printf("term_handler\n");
  if(sig==SIGINT)
  {
   close (sockfd);
   exit(0);
  }
}

//==============================================================================
//Read strings from file
char * readDataFromFile(char * filename)
{
	FILE * sourceFile;
	char * validFilename;
	char * currentStr;
	char * data;
	char sendBuff[1024];
	
	memset(sendBuff, '0', sizeof(sendBuff));

	sendBuff[0] = '\0';

	currentStr 	  = malloc(20);
	
	sourceFile  = fopen(filename, "r");

	while(fgets(currentStr, 20, sourceFile))
	{
		strcat(sendBuff, currentStr);
	}

	strcat(sendBuff, "\0");

	free(currentStr);
	fclose(sourceFile);

	return sendBuff;
}

//==============================================================================
//Write strings to file
void writeDataToFile(char * sourceFilename, char * data)
{
	char * validFilename;
	FILE * validFile;
	char   currentStr[20];
	int    i=0;
	char 	textSize[4];

	validFilename = malloc(20);

	memset(currentStr, '\0',strlen(currentStr));
	memset(textSize, '\0',strlen(textSize));

	strcpy(validFilename, sourceFilename);
	strcat(validFilename, ".valid");

	validFile = fopen(validFilename, "w");

	//get size of recive text
	for(i=0;data[i]!='\n'&&i<4;i++)
	{
		textSize[i] = data[i];
	}

	//parse and save
	for(i=3;i<atoi(textSize)+3;i++)
	{
		if(data[i]!='\n')
		{
			strncat(currentStr,&data[i],1);
		}
		else
		{
			strcat(currentStr,&"\n");

			fputs(currentStr,validFile);
			
			memset(currentStr, '\0',sizeof(currentStr));
		}
	}

	free(validFilename);

	fclose(validFile);
}

//==============================================================================
//Connect to listener and recieve port to work
int  get_work_port()
{
	struct sockaddr_in serv_addr;
	char port[5];

	memset(port, '0', sizeof(port));

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);
    inet_pton(AF_INET, IP, &serv_addr.sin_addr);

    //this delay may be require to initialize listener on server
    sleep(2);

    //connect to server-listener
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
    	printf("can not get work port\n");
    }

    //get port number
    read(sockfd, port, sizeof(port));

    shutdown(sockfd,2);
    close(sockfd);

    return atoi(port);
}

//==============================================================================
//Read data from files, send data to server, recieve data, write results to file
void client(int argc, char ** argv)
{
    char    recvBuff[1024];
	int     i;
	int     filesCount;
	int     work_port;
    struct  sockaddr_in serv_addr; 
    char *  sourceData;

	filesCount = (argc-1)/2;

	work_port = get_work_port();

	printf("work on %d port\n", work_port);

	memset(recvBuff, '0', sizeof(recvBuff));

	//socket initialize
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(work_port);
    inet_pton(AF_INET, IP, &serv_addr.sin_addr);

	for(i=1;i <= filesCount;i++)
	{
		if (!strcmp(argv[i*2],"-f"))
		{
			sourceData = readDataFromFile(argv[i*2+1]);
			//connect initialize
			if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
			{
				printf("can not connect to server");
				close(sockfd);
				return 1;
			}

			if(write(sockfd, sourceData, strlen(sourceData))<0)
			{
				printf("can not write to connection");
				return 1;
			}
			if (read(sockfd, &recvBuff,strlen(recvBuff))<0)
			{
				printf("can not read from connection");
				return 1;
			}

			shutdown(sockfd,2);
			close(sockfd);
			
			writeDataToFile(argv[i*2+1], recvBuff);
		}
	}

	printf("Succesfully validate\n");
}

#endif //CLIENT_H