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