#ifndef PARSER_H
#define PARSER_H

struct InputArgs parseInputCommand(char * inputCommand)
{
	struct InputArgs args;
	long i;
	args.argc = 0;
	long    firstSym=0;

	for(i=strlen(inputCommand)-1;i>0;i--)
	{
		if(inputCommand[i]==' ')
		{
			inputCommand[i]='\0';
		}
		else
		{
			break;
		}
	}

	for(i=0;i<strlen(inputCommand);i++)
	{
		if(inputCommand[i]!=' ')
		{
			firstSym = i;
			break;
		}
	}

	if(firstSym==strlen(inputCommand)-1||strlen(inputCommand)==0)
	{
		return args;
	}

	for(i=firstSym;i<strlen(inputCommand)-1;i++)
	{
		if((inputCommand[i]!=' '&&inputCommand[i+1]==' ')||(i==strlen(inputCommand)-1))
		{
			args.argc++;
		}
	}

	// if(args.argc==0)
	// 	return args;

	args.argc++;

	args.argv = malloc(args.argc);
	long argNumber=0;
	char tmpStr[255]={'\0'};

	for(i=firstSym;i<=strlen(inputCommand);i++)
	{
		if((inputCommand[i]!=' ')&&(i!=strlen(inputCommand)))
		{
			strncat(tmpStr,&inputCommand[i],1);
		}
		else
		{
			if(((inputCommand[i]==' ')&&(inputCommand[i-1]!=' '))||((inputCommand[i]!=' ')&&(i==strlen(inputCommand))))
			{
				args.argv[argNumber] = malloc(strlen(tmpStr));
				strcpy(args.argv[argNumber],tmpStr);
				argNumber++;
				memset(tmpStr, '\0', strlen(tmpStr));
			}
		}
	}

	return args;
}

#endif //PARSER_H