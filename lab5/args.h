#ifndef ARGS_H
#define ARGS_H

#include <sys/types.h>

struct Processes
{
	int   	p_number;
	int   	p_low_limit;
	int   	p_top_limit;
	pid_t   pid;
	int 	fifofd;
};

#endif //ARGS_H