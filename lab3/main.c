#include <stdio.h>
#include <regex.h>
#include <string.h>

//==============================================================================
//Return 1 if string matches the regular expression (pattern)
int match(const char *string, char *pattern)
{
    int    status;
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
int main(int argc, char ** argv)
{
	if((argc < 4)||(strcmp(argv[1],"-v"))||(strcmp(argv[3],"-i")))
	{
		printf("Please enter filename. Use format: -v <file_with_valid> -i <file_with_invalid>\n");
		return 1;
	}

	char    inputStr[20];
	int     validFile;
	int     invalidFile;
	int     spaceLines=0;
	int 	validEmails=0;//Count of valid and invalid emails
	int 	invalidEmails=0;

	validFile = fopen(argv[2],"w");
	invalidFile = fopen(argv[4],"w");

	while(spaceLines < 2)
	{
		gets(inputStr);

		if(!strlen(inputStr))
		{
			spaceLines++;
		}
		else
		{
			inputStr[sizeof(inputStr)] = "\0";

			if(match(inputStr, "^[a-zA-Z0-9]+@[a-zA-Z0-9]+[.][a-zA-Z0-9]+$"))
			{
				validEmails++;
				fputs(inputStr, validFile);
				fputs("\n", validFile);
				printf("This is valid email");
			}
			else
			{
				invalidEmails++;
				fputs(inputStr, invalidFile);
				fputs("\n", invalidFile);
				printf("This is invalid email");
			}
		}
	}

	fclose(validFile);
	fclose(invalidFile);

	printf("Total count of valid emails: %d\n", validEmails );
	printf("Total count of invalid emails: %d\n", invalidEmails );

	exit(0);
}