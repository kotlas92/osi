#define __USE_LARGEFILE64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "types.h"
#include "parser.h"
#include "info.h"
#define MAX_FILES_COUNT 1024

#define REF_SIZE 20

long stat_block_start;
long fat_block_start;


//==============================================================================
long format(int blockSize)
{
	struct stat st;
	char   data[100]={'\0'};
	char   tmpChar[20];
	long    i_block_count;
	long    i_data_blocks_count;
	char   blocksCount[10];
	long    i_data_start = 50 + MAX_FILES_COUNT*256;//fake
	char   c_data_start[20];
	long    i_stat_block_size;
	char * c_stat_blocks;
	long    superblock_size = 100;
	long    i_fat_size = MAX_FILES_COUNT*(max_filename_size + sizeof(long) + sizeof(long) + sizeof(long));
	char   c_stat_block_start[10];
	char   c_fat_block_start[10];
	char   block_size[10];

	int fd;
	fd = open(FILENAME, O_RDWR );

  	stat64(FILENAME, &st);

  	i_data_blocks_count = ((st.st_size-superblock_size-i_fat_size)/(blockSize+0.125));

  	i_stat_block_size = i_data_blocks_count/8;

  	stat_block_start = superblock_size;
  	fat_block_start  = stat_block_start + i_stat_block_size;
  	i_data_start     = fat_block_start  + i_fat_size;

  	i_block_count = i_data_blocks_count;

  	sprintf(tmpChar,"%020d",st.st_size);
  	//One byte stat-blocks need to every 8 blocks of data
  	sprintf(blocksCount, "%010d",i_block_count);
  	sprintf(block_size, "%010d",blockSize);
  	sprintf(c_stat_block_start, "%010d", stat_block_start);
  	sprintf(c_fat_block_start, "%010d", fat_block_start);
  	sprintf(c_data_start, "%010d",i_data_start);

  	sprintf(data,"%s%s%s%s%s%s",tmpChar,blocksCount,block_size,c_stat_block_start,c_fat_block_start, c_data_start);

  	if(write(fd,data,strlen(data))<0)
  	{
  		printf("can not write to file\n");
  		return 1;
  	}

  	char * null_fat;

  	null_fat = malloc(i_fat_size);

  	memset(null_fat, '\0', i_fat_size);

	c_stat_blocks = malloc(i_block_count);
	memset(c_stat_blocks, '\0', i_block_count);

  	write(fd,c_stat_blocks,strlen(c_stat_blocks));

  	lseek(fd,fat_block_start,SEEK_SET);

  	if(write(fd,null_fat,i_fat_size)<0)
  	{
  		printf("can not write to file\n");
  		return 1;
  	}

  	close(fd);

  	printf("formatting completed\n");

  	free(c_stat_blocks);

  	return 0;
}

//==============================================================================

int copy_new_file(char * filename)
{
	struct stat st;
	stat64(filename, &st);
	long fat_record_offset = get_free_fat_record();
	long blocks_count,fd_inp,fd_out,i,data_start_offset,block_size;
	char * buf;

	buf = malloc(get_block_size());

	if(st.st_size%(get_block_size()-REF_SIZE))
	{
		blocks_count = st.st_size/(get_block_size()-REF_SIZE)+1;
	}
	else
	{
		blocks_count = st.st_size/(get_block_size()-REF_SIZE);
	}

	int * free_blocks 	  = get_free_block_number(blocks_count,'0');

	fd_inp = open(filename, O_RDWR );

	fd_out = open(FILENAME, O_RDWR );
	
	data_start_offset = get_data_block_start();
	block_size = get_block_size();

	for(i=0;i<blocks_count;i++)
	{
		char next_block[REF_SIZE];
		read(fd_inp,buf,block_size-REF_SIZE);
		lseek(fd_out,data_start_offset+block_size*free_blocks[i],SEEK_SET);
		write(fd_out,buf,block_size);

		lseek(fd_out,data_start_offset+block_size*free_blocks[i]+block_size-REF_SIZE,SEEK_SET);
		
		if(i+1<blocks_count)
		{
			sprintf(next_block, "%020d", free_blocks[i+1]);
		}
		else
		{
			sprintf(next_block, "%020d", 0);
		}

		write(fd_out,next_block,block_size);
	}

	printf("%d blocks copied\n", blocks_count);

	if(lock_blocks_stat(free_blocks, blocks_count))
	{
		printf("can not change stats");
		return 1;
	}

	lseek(fd_out,fat_record_offset,SEEK_SET);

	char * fat_record;

	fat_record = malloc(get_fat_size());

	memset(fat_record, '\0', strlen(fat_record));

	char filesize[8];
	char filesize_blocks[8];
	char first_block[8];

	sprintf(filesize, "%08d", st.st_size);
	sprintf(filesize_blocks, "%08d", blocks_count);
	sprintf(first_block, "%08d", free_blocks[0]);

	write(fd_out, filename, max_filename_size);
	lseek(fd_out,fat_record_offset+max_filename_size,SEEK_SET);	
	strcat(fat_record, filesize);
	strcat(fat_record, filesize_blocks);
	strcat(fat_record, first_block);

	write(fd_out, fat_record, get_fat_record_size());

	free(buf);
	free(fat_record);

	close(fd_out);
	close(fd_inp);
	return 0;
}

//==============================================================================
int cat(char * filename)
{
	char * fat_record = get_fat_record(filename);

	if(*fat_record=='\0')
	{
		printf("file not found!\n");
		return 0;
	}
	//only if file exist
	int next_block_number = get_first_block_number(filename);

	while(next_block_number!=0)
	{
		printf("%s", get_block_data(next_block_number) );
		next_block_number = get_next_block_number(next_block_number);
	}
	printf("\n");
	return 0;
}

//==============================================================================

int copy(char * filename)
{
	struct stat st;
	int res;

	if(stat64(filename, &st)==-1)
	{
		printf("file not exist\n");
		return 1;
	}

	char * fat_record = get_fat_record(filename);

	//if it is new file
	if(*fat_record!='\0')
	{
		rm(filename);	
	}

	res = copy_new_file(filename);

	return 0;
}

//==============================================================================
int * get_file_blocks(char * filename)
{
	int blocks_count = get_size_in_blocks(filename);
	int * blocks;
	blocks = malloc(blocks_count*sizeof(int));
	int i=0;
	int next_block_number = get_first_block_number(filename);

	while(next_block_number!=0)
	{
		blocks[i] = next_block_number;
		i++;
		next_block_number = get_next_block_number(next_block_number);
	}
	return blocks;
}

int clear_fat_record(char * filename)
{
	char * null_record;
	memset(null_record, '\0', get_fat_record_size());
	int fd = open(FILENAME, O_RDWR);
	lseek(fd, get_fat_record_offset(filename), SEEK_SET);
	write(fd, null_record, get_fat_record_size());
	close(fd);
	return 0;
}

//==============================================================================

int rm(char * filename)
{
	char * fat_record = get_fat_record(filename);
	int i;

	if(!fat_record)
	{
		printf("file not exist\n");
		return 1;
	}

	int blocks_count = get_size_in_blocks(filename);

	int * blocks = get_file_blocks(filename);
	
	unlock_blocks_stat(blocks, blocks_count);

	clear_fat_record(filename);
	
	return 0;
}

//==============================================================================

int ls()
{
	long fat_size = get_fat_size();
	long record_size = get_fat_record_size();
	long records_count = fat_size/record_size;
	long fat_start = get_fat_block_start();

	char * fat;

	fat = get_symbols(fat_size, fat_start);

	int i;

	for(i=0;i<records_count;i++)
	{
		char * buf;
		buf = get_symbols(max_filename_size, fat_start+i*record_size);
		if(*buf!='\0')
		{
			printf("%s\n", buf);
		}
	}
	
	free(fat);
	return 0;
}

//==============================================================================

int main(int argc, char ** argv)
{
	char inputCommand[100];

	printf("Welcome to super-fs:)\n");

	// while(1)
	// {
		struct InputArgs args;
		printf("$");

		gets(inputCommand);

		char buf;

		args = parseInputCommand(inputCommand);

		if(args.argc!=0)
		{
			if(!strcmp(args.argv[0], "exit"))
			{
				printf("Buy\n");
				return 0;
			}

			if(!strcmp(args.argv[0], "format"))
			{
				if((args.argc==3)&&(!strcmp(args.argv[1],"-b"))&&(atoi(args.argv[2])>512))
				{
					format(atoi(args.argv[2]));
				}
				else
				{
					format(512);
				}
			}
			if(!strcmp(args.argv[0], "cp")&&(args.argc==2))
			{
				copy(args.argv[1]);
			}
			if(!strcmp(args.argv[0], "cat")&&(args.argc==2))
			{
				cat(args.argv[1]);
			}
			if(!strcmp(args.argv[0], "rm")&&(args.argc==2))
			{
				rm(args.argv[1]);
			}
			if(!strcmp(args.argv[0], "ls"))
			{
				ls();
			}
		}
		memset(inputCommand,'\0',strlen(inputCommand));
		
	// }

	return 0;
}
