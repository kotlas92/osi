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
#include <math.h>

void start_worker(long i)
{
	char pathname[40];
	long  fifofd;
	char p_number[4];
	char range[40];
	char c_low_limit[20];
	char c_top_limit[20];
	long  n_top_low_flag=0;
	char * primes;

	memset(pathname, '\0', strlen(pathname));
	memset(range, '\0', strlen(range));
	memset(c_top_limit, '\0', strlen(c_top_limit));
	memset(c_low_limit, '\0', strlen(c_low_limit));

	strcat(pathname,"worker\0");
	sprintf(p_number,"%d",i);
	strcat(pathname,p_number);

	fifofd = open(pathname,O_RDWR);

	read(fifofd, range, 40);

	primes = malloc(sizeof(char)*atoi(range)*30);
	memset(primes, '\0', strlen(primes));

	for(i=0;i<strlen(range);i++)
	{
		if(range[i]==':')
		{
			n_top_low_flag = 1;
			i++;
		}
		if(n_top_low_flag==0)
		{
			strncat(c_low_limit,&range[i],1);
		}
		else
		{
			strncat(c_top_limit,&range[i],1);
		}
	}

	for(i=atol(c_low_limit);i<=atol(c_top_limit);i++)
	{
		long j;
		long dividers=0;
		// to optimize uncomment three lines and comment lines "for"
		// long max_i;
		// max_i = pow(i,0.5);
		// for(j=1;j<max_i;j++)
		for(j=1;j<i;j++)
		{
			if((i%j==0)&&(j!=1)&&(j!=i))
			{
				dividers++;
				//to optimize uncomment line ""
				// break;
			}
		}
		if(dividers==0)
		{
			printf("%d\n", i);
			char temp_char[10];
			sprintf(temp_char,"%d\n\0",i);
			strcat(primes,temp_char);
		}
	}

	primes[i+1]='\0';

	write(fifofd,primes,strlen(primes));
}

long main(long argc, char ** argv)
{
	
	char    c_low_limit[10];
	char    c_top_limit[10];
	long     n_low_limit;
	long    	n_top_limit;
	long 	n_top_low_flag=0;//only to parse incoming parameters
	long 	residue;

	struct Processes processes[10];

	memset(c_low_limit, '\0', strlen(c_low_limit));
	memset(c_top_limit, '\0', strlen(c_top_limit));
	
	if(argc != 5)
	{
		printf("Error. Use format: \n    -p <process_count> -r <low_limit>:<top_limit>\n");
		return 1;
	}

	long i=1;

	for(i=0;i<strlen(argv[4]);i++)
	{		
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
		char p_number[4];

		memset(range, '\0', strlen(range));
		memset(temp_string, '\0', strlen(temp_string));
		memset(pathname, '\0', strlen(pathname));
		memset(p_number, '\0', strlen(p_number));

		strcat(pathname,"worker\0");
		
		sprintf(p_number,"%d", i);

		strcat(pathname,p_number);
		
		if (mkfifo(&pathname, S_IRUSR|S_IWUSR))
		{
			printf("Can not create FIFO");
			return 1;
		}

		processes[i].p_low_limit = ((n_top_limit-residue-n_low_limit)/atoi(argv[2]))*i+n_low_limit;

		processes[i].p_top_limit = ((n_top_limit-residue-n_low_limit)/atoi(argv[2]))+processes[i].p_low_limit;

		processes[i].fifofd = open(pathname,O_RDWR);

		if(processes[i].fifofd==-1)
		{
			printf("Could not open named pipe");
			return 1;
		}

		sprintf(temp_string, ":%d", processes[i].p_top_limit);

		sprintf(range, "%d", processes[i].p_low_limit);

		strcat(range, temp_string);

		write(processes[i].fifofd,range,strlen(range));

		processes[i].p_number = i;
		
		processes[i].pid = fork();

		if(processes[i].pid==0)
		{
			start_worker(i);
			return 0;
		}
		else
		{
			printf("created new proccess with id %d\n", processes[i].pid );
			printf("range: %s\n", range);
		}
	}

	for(i=0;(i<atoi(argv[2]))&&processes[0].pid;i++)
	{
		char p_number[5];
		char buff[100000];
		char pathname[20];
		memset(buff,'\0',strlen(buff));
		memset(pathname, '\0', strlen(pathname));
		printf("wait to complete pid %d\n", processes[i].pid );
		waitpid(processes[i].pid,NULL,NULL);		
		read(processes[i].fifofd,buff,100000 );
		printf(buff);
		strcpy(pathname,"worker\0");
		sprintf(p_number,"%d",i);
		strcat(pathname, p_number);
		unlink(pathname);
	}

	return 0;
}