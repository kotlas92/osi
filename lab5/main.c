#include <stdio.h>
#include <unistd.h>
#include <regex.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <signal.h>
#include "args.h"
#include <fcntl.h>
#include <sys/stat.h>

void start_worker(int i)
{
	char pathname[40];
	int  fifofd;
	char p_number[4];
	char range[40];
	char c_low_limit[20];
	char c_top_limit[20];
	int  n_top_low_flag=0;
	char * primes;

	memset(pathname, '\0', strlen(pathname));
	memset(range, '\0', strlen(range));

	strcat(pathname,"/tmp/worker");
	sprintf(p_number,"%d",i);
	strcat(pathname,p_number);

	fifofd = open(&pathname,O_RDWR);

	read(fifofd, range, strlen(range));

	primes = malloc(sizeof(char)*atoi(range));
	memset(primes, '\0', strlen(primes));

	for(i=0;i<strlen(range);i++)
	{
		if(range[i]!=":")
		{
			n_top_low_flag = 1;
		}
		if(n_top_low_flag==0)
		{
			strcat(c_low_limit,range[i]);
		}
		else
		{
			strcat(c_top_limit,range[i]);
		}
	}

	for(i=atoi(c_low_limit);i<atoi(c_top_limit);i++)
	{
		int j;
		int dividers=0;
		for(j=1;j<i;j++)
		{
			if((i%j==0)&&(j!=1)&&(j!=i))
			{
				dividers++;
			}
		}
		if(dividers==2)
		{
			char temp_char[10];
			sprintf(temp_char,"%d\n\0",i);
			strcat(primes,temp_char);
		}
	}

	write(fifofd,primes,strlen(primes));
}

int main(int argc, char ** argv)
{
	
	char    c_low_limit[10];
	char    c_top_limit[10];
	int     n_low_limit;
	int    	n_top_limit;
	int 	n_top_low_flag=0;//only to parse incoming parameters
	int 	residue;

	struct Processes * processes;

	memset(c_low_limit, '\0', strlen(c_low_limit));
	memset(c_top_limit, '\0', strlen(c_top_limit));
	
	if(argc != 5)
	{
		printf("Error. Use format: \n    -p <process_count> -r <low_limit>:<top_limit>\n");
		return 1;
	}

	processes = malloc(sizeof(struct Processes)*atoi(argv[2]));

	int i=1;

	for(i=0;i<strlen(argv[4]);i++)
	{
		printf("asdasd");
		
		if(argv[4][i]!=':')
		{
			if(n_top_low_flag==0)
			{
				strncat(c_low_limit, &argv[4][i],1);
			}
			else
			{
				strncat(c_top_limit, &argv[4][i],1);
			}
		}
		else
		{
			n_top_low_flag = 1;
			n_low_limit = atoi(c_low_limit);
		}
	}

	n_top_limit = atoi(c_top_limit);

	if((n_top_limit - n_low_limit) % atoi(argv[2])==0)
	{
		residue = (n_top_limit - n_low_limit) % atoi(argv[2]);
	}
	else
	{
		residue = 0;
	}

	for(i=0;i<atoi(argv[2])&&getppid()!=0;i++)
	{
		char pathname[40];

		char range[40];
		char temp_string[10];

		memset(range, '\0', strlen(range));
		memset(temp_string, '\0', strlen(temp_string));
		memset(pathname, '\0', strlen(pathname));

		strcat(pathname,"worker\0");
		
		strcat(pathname,&i);
		
		mkfifo(&pathname, O_RDWR);

		processes[i].p_low_limit = ((n_top_limit-residue)/atoi(argv[2]))*i;

		processes[i].p_top_limit = ((n_top_limit-residue)/atoi(argv[2]))+processes[i].p_low_limit;

		processes[i].fifofd = open(&pathname,O_RDWR);

		sprintf(temp_string, ":%d", processes[i].p_top_limit);

		sprintf(range, "%d", processes[i].p_low_limit);

		strcat(range, temp_string);

		write(processes[i].fifofd,range,strlen(range));
		
		processes[i].pid = fork();
		
		if(getppid()!=0)
		{
			start_worker(i);
		}
	}

	for(i=0;i<argv[2];i++)
	{
		char buff[1000];
		memset(buff,'\0',strlen(buff));
		waitpid(processes[i].pid,NULL,NULL);
		read(processes[i].fifofd,buff,strlen(buff));
		printf(buff);
		kill(processes[i].pid, SIGINT);
	}

	return 0;
}