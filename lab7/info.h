#ifndef INFO_H
#define INFO_H
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
#define MAX_FILES_COUNT 1024

#define REF_SIZE 20

const char FILENAME[]="diskfile.img";
const int max_filename_size = 255;
const int size_in_blocks = 8;
const int size_in_bytes  = 8;
const int block_pointer_size = 8;

//==============================================================================
char * get_symbols(int count, int offset)
{
	int fd;
	char * buf;
	buf = malloc(count);
	fd = open(FILENAME,O_RDWR);
	lseek(fd, offset, SEEK_SET);
	read(fd, buf, count);
	close(fd);
	return buf;
}

//==============================================================================
long get_block_size()
{
	return atoi(get_symbols(10,30));
}

//==============================================================================
int get_filesize()
{
	return atoi(get_symbols(20,0));
}

//==============================================================================
int get_blocks_count()
{
	return atoi(get_symbols(10,20));
}

//==============================================================================
long get_stat_block_start()
{
	//stopgap
	return atoi(get_symbols(10,40))+2;
}

//==============================================================================
int get_fat_block_start()
{
	return atoi(get_symbols(10,50));
}

//==============================================================================
int get_data_block_start()
{
	return atoi(get_symbols(10,60));
}

//==============================================================================
int get_data_blocks_count()
{
	return atoi(get_symbols(10,20));
}

//==============================================================================
char * get_stat()
{
	return get_symbols(get_blocks_count(),get_stat_block_start());
}

//==============================================================================
int write_stat(char * buf)
{
	int blocksCount = get_blocks_count();
	int fd;
	fd = open(FILENAME, O_RDWR );
	
	lseek(fd, get_stat_block_start(), SEEK_SET);
	
	write(fd, buf, blocksCount);
	
	close(fd);
	return 0;
}

//==============================================================================
char * get_fat()
{
	return get_symbols(get_fat_size(),get_fat_block_start());
}

//==============================================================================
int get_fat_size()
{
	return MAX_FILES_COUNT*(get_fat_record_size());
}

//==============================================================================
long get_free_fat_record()
{
	return get_fat_record_offset(&"");
}

//==============================================================================
int get_fat_record_size()
{
	return max_filename_size + sizeof(long) + sizeof(long) + sizeof(long);
}
//==============================================================================
int get_fat_record_offset(char * filename)
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
		if(!strcmp(filename, buf))
		{
			return fat_start+i*record_size;
		}
	}
	
	free(fat);
	return &"0";
}

//==============================================================================
char * get_fat_record(char * filename)
{
	int offset;

	offset = get_fat_record_offset(filename);
	
	if(offset!='0')
	{
		return get_symbols(get_fat_record_size(), offset);
	}
	else
	{
		return 0;
	}
}
//==============================================================================
int ipow(int number, int degree)
{
	int i;
	int res=number;
	if(degree==0)
	{
		return 1;
	}
	for(i=1;i<degree;i++)
	{
		res*=number;
	}
	return res;
}
//==============================================================================
int * get_free_block_number(int block_count, char c_fl_full_size)
{
	int fd;
	unsigned char * buf;

	char c_stat_block_start[10];
	int  * result;

	buf = get_stat();

	int i;
	int total_block_count = get_data_blocks_count();
	int block_number=0;
	result = malloc(sizeof(int)*block_count);

	memset(result, '\0', (sizeof(int)*block_count));

	for(i=0;i<total_block_count && block_number < block_count;i++)
	{
		if(buf[i]!=255)
		{
			int j;
			for(j=1;j<=8;j++)
			{
				if((int)buf[i]+1==ipow(2,j-1))
				{
					result[block_number] = i*8+j;
					buf[i]+=ipow(2,j-1);
					block_number++;
				}

				if(block_number==block_count)
				{
					break;
				}
			}
		}
	}
	if(block_number<block_count-1)
	{
		result[0] = -1;
	}

	free(buf);

	return result;
}

//==============================================================================
int lock_blocks_stat(int * blocks, int count)
{
	return change_blocks_stat(blocks,count, 1);
}

//==============================================================================
int change_blocks_stat(int * blocks, int count, int new_block_state)
{
	int i;
	unsigned char * stat;

	stat = get_stat();
	for(i=0;i<count;i++)
	{
		int stat_superblock_number = blocks[i]/8;
		if(blocks[i]%8)
		{
			stat_superblock_number+=1;
		}
		int stat_block_number = blocks[i] - (stat_superblock_number-1)*8 - 1;
		if(new_block_state)
		{
			stat[stat_superblock_number-1] += ipow(2,stat_block_number);
		}
		else
		{
			stat[stat_superblock_number-1] -= ipow(2,stat_block_number);
		}
	}
	
	return write_stat(stat);
}

//==============================================================================
int unlock_blocks_stat(int * blocks, int count)
{
	return change_blocks_stat(blocks,count, 0);
}

//==============================================================================
int get_first_block_number(char * filename)
{
	int offset = get_fat_record_offset(filename)+max_filename_size+size_in_bytes+size_in_blocks;
	return atoi(get_symbols(block_pointer_size, offset));
}

//==============================================================================
int get_size_in_blocks(char * filename)
{
	int offset = get_fat_record_offset(filename)+max_filename_size+size_in_bytes;
	return atoi(get_symbols(size_in_blocks, offset));
}

//==============================================================================
int get_size_in_bytes(char * filename)
{
	int offset = get_fat_record_offset(filename)+max_filename_size;
	return atoi(get_symbols(size_in_bytes, offset));
}

//==============================================================================
char * get_block_data(int block_number)
{
	int data_block_start = get_data_block_start();
	int block_size = get_block_size();

	return get_symbols(block_size-REF_SIZE, data_block_start+block_size*(block_number));
}

//==============================================================================
int get_next_block_number(int block_number)
{
	int data_block_start = get_data_block_start();
	int block_size = get_block_size();

	return atoi(get_symbols(REF_SIZE, data_block_start+block_size*(block_number)+block_size-REF_SIZE));
}
#endif //INFO_H