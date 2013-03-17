#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>


//==============================================================================
//Return 1 if string matches the regular expression (pattern)
int match(const char *string, char *pattern)
{
    int        status;
    regex_t    re;

    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
      return(0);
    }
    status = regexec(&re, string, (size_t) 0, NULL, 0);
    regfree(&re);
    if (status != 0) {
   	  return(0);
    }
    return(1);
};

//==============================================================================
//This function read strings from file, check mathes to regex and write 
//valid and invalid to appropriate output files
void * validate(char * filename)
{
	printf("Created new thread with id: %d\n",syscall (SYS_gettid));

	FILE * validFile;
	FILE * invalidFile;
	FILE * sourceFile;
	char * validFilename;
	char * invalidFilename;
	char * currentStr;
	
	validFilename 	= malloc(80);
	invalidFilename = malloc(80);
	currentStr 		= malloc(20);

	memcpy(validFilename,   filename,strlen(filename));
	memcpy(invalidFilename, filename,strlen(filename));
	
	validFile   = fopen(strcat(validFilename,   ".valid"  ),"w");
	invalidFile = fopen(strcat(invalidFilename, ".invalid"),"w");
	sourceFile  = fopen(filename, "r");
	
	while(fgets(currentStr, 20, sourceFile))
	{
		int i;
		for(i=0;i<strlen(currentStr);i++)
		{
			if (currentStr[i]=='\n')
			{
				currentStr[i]='\0';
			}
		}
		
		if(match(currentStr, "^[a-zA-Z0-9]+@[a-zA-Z0-9]+[.][a-zA-Z0-9]+$"))
		{
			fputs(currentStr, validFile);
			fputs("\n", validFile);
		}
		else
		{
			fputs(currentStr, invalidFile);
			fputs("\n", invalidFile);	
		}
	}

	fclose(sourceFile);
	fclose(validFile);
	fclose(invalidFile);
	
	free(validFilename);
	free(invalidFilename);
	free(currentStr);

	printf("Thread with id %d complete work\n",syscall (SYS_gettid));
}

int main(int argc, char ** argv)
{
	if((argc < 3)||(strcmp(argv[1],"-f"))||((argc-1)%2))
	{
		printf("Please enter filename. Use format: -f <file_with_data>\n");
		return 1;
	}

	int    		i;//counter to creating threads
	int    		filesCount;
	pthread_t 	thread_id[10];

	filesCount = (argc-1)/2;

	//Open new threads to each file
	for(i=0;i <= filesCount;i+=2)
	{
		if (!strcmp(argv[i+1],"-f"))
		{
			pthread_create( &thread_id[i], NULL, &validate, (void*) argv[i+2]);
		}
	}

	//Join opened threads
	for(i=0;i <= filesCount;i+=2)
	{
		if (!strcmp(argv[i+1],"-f"))
		{
			pthread_join(thread_id[i], NULL);
		}
	}

	return 0;
}