#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "error.h"

struct ErrorType *error_first;

void error_init(const char *programName, int defaultExitCode, int *failureIndicator)
{
	/*
	**  This function inits the error module. I did not find any proper
	**  way to init the value of error_first without calling such a function.
	*/

	// Initializes static values.
	error_static(0,programName,defaultExitCode,failureIndicator);

	// Inits error_first's value.
	error_first = NULL;
}

void error_add(int level, const char *name, const char *format)
{
	/*
	**  This function adds an error type that can be used immediatly.
	**  Every error type must be defined befor being used.
	*/

	// Allocating a new error type;
	struct ErrorType *type = malloc(sizeof(struct ErrorType));
	struct ErrorType *parser = error_first;

	// If no error type exists, creates the first.
	if(!error_first) error_first = type;

	// Else, adds it at the end of the linked list.
	else
	{
		while(parser->next) parser = parser->next;
		parser->next = type;
	}

	// Initializes the error type depending of the given parameters.
	type->maskable = 0;
	if(*name=='~' && *(name+1)) name++, type->maskable=1;
	type->level = level;
	type->name = strcpy(malloc(strlen(name)+1),name);
	type->format = strcpy(malloc(strlen(format)+1),format);
	type->activated = 1;
	type->next = NULL;
}

int error_argument(const char *argument)
{
	/*
	**  This function analyses an argument string to active or de-activate
	**  error types. 
	*/

	// Activation mode, level indicator, return value, linked list parser;
	int activate = 1, level = 0, x = 1;
	struct ErrorType *parser = error_first;

	// Argument format : -[N|W|E][all|[no-]<error_name>]
	if(*argument++!='-') return 1;
	switch(*argument)
	{
		case 'N': level = NOTE; break;
		case 'W': level = WARNING; break;
		case 'E': level = ERROR; break;
		default : return 1;
	}

	if(!*++argument) return 1;
	if(!strncmp(argument,"no-",3)) activate=0, argument+=3;
	if(!*argument) return 1;

	do
	{
		if(parser->level==level && !strcmp(parser->name,argument) && parser->maskable)
			parser->activated = activate, x = 0;
	} while((parser = parser->next));

	return x;
}

void error_emit(int level, const char *name, ...)
{
	/*
	**  This function outputs an error on stdout.
	*/

	// Declarging a linked list parser and the args list.
	struct ErrorType *parser = error_first;
	va_list args;

	// Starting the va_list to give to printf().
	va_start(args,name);

	// Parsing the defined error types to match the given one.
	do
	{
		// Testing if parser matches the same error as the given one.
		if(parser->level==level && !strcmp(parser->name,name) && parser->activated)
		{
			error_static(1,NULL,0,NULL);

			switch(parser->level) {
				case FATAL: printf("fatal error: "); break;
				case ERROR: printf("error: "); break;
				case WARNING: printf("warning: "); break;
				case NOTE: printf("note: "); break; }
			vprintf(parser->format,args);
			printf("\n");

			if(level==ERROR) error_static(3,NULL,0,NULL);
			if(level==FATAL) error_static(2,NULL,0,NULL);
		}
	}
	// Note that is it absolutely possible to use several error messages.
	while((parser = parser->next));

	// Ending the argument list.
	va_end(args);
}

void error_static(int x, const char *p, int e, int *f)
{
	/*
	**  This function handles the user program name and the default exit
	**  code for fatal errors.
	*/

	// Static variables.
	static int defaultExitCode = 1;
	static const char *programName = NULL;
	static int *failureIndicator = NULL;

	// Selecting action.
	switch(x)
	{
		// Initializes parameters.
		case 0:
			if(p) programName = strcpy(malloc(strlen(p)+1),p);
			defaultExitCode = e;
			if(f) failureIndicator = f;
			break;

		// Prints program name.
		case 1:
			if(programName) printf("%s: ",programName);
			break;

		// Exits program.
		case 2:
			exit(defaultExitCode);

		// Set failure indicator.
		case 3:
			*failureIndicator = 1;
			break;
	}
}
