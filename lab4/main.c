#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <pthread.h>

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

void * validate(char * filename)
{
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
}

int main(int argc, char ** argv)
{
	if((argc < 3)||(strcmp(argv[1],"-f"))||((argc-1)%2))
	{
		printf("Please enter filename. Use format: -f <file_with_data>\n");
		return 1;
	}

	int    i;//counter to creating threads
	int    filesCount;
	pthread_t thread_id[10];

	filesCount = (argc-1)/2;

	//open threads
	for(i=1;i <= filesCount;i+=2)
	{
		if (!strcmp(argv[i],"-f"))
		{
			pthread_create( &thread_id[i], NULL, &validate, (void*) argv[i+1]);
		}
	}

	//join threads
	for(i=1;i <= filesCount;i+=2)
	{
		if (!strcmp(argv[i],"-f"))
		{
			pthread_join(thread_id[i], NULL);
		}
	}

	printf("Succesfully validate\n");

	return 0;
}