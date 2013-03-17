#ifndef ARGS_H
#define ARGS_H

//This structure used for maintenance thread-pull in actual state
struct ThreadArgs
{
	int * 	pull;//This pointer used to save thread status (busy or free)
	int 	currentId;//This is not system threadID. 
					  //This id used to calculate port to work
};

#endif //ARGS_H