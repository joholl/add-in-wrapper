/*
	Header inclusions.
*/

// Standard headers.
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// Module header.
#include "error.h"



/*
	Composed types definitions.

	These types are used only in this file.
*/

// ErrorType linked list node.
struct Error
{
	// Error level.
	enum Error_Level level;

	// Activated and maskable flags.
	int activated :4;
	int maskable  :4;

	// Error name and format.
	const char *name;
	const char *format;

	// Linked list pointer.
	struct Error *next;
};



/*
	Static variables definitions.
*/

// Linked list first node.
static struct Error *error_first = NULL;
// Error display prefix (usually program name).
static const char *prefix;
// Exit code on fatal error.
static int exit_code;
// Failure indicator.
static int *failure;



/*
	Function definitions.
*/

/*
	error_init()

	Initializes the error module.

	@arg	program_name		Program name, shown before error.
	@arg	default_exit_code	Exit code on fatal error.
	@arg	failure_indicator	Integer pointer set on failure.
*/

void error_init(const char *program_name, int default_exit_code,
	int *failure_indicator)
{
	// Initializing static values.
	prefix = program_name;
	exit_code = default_exit_code;
	failure = failure_indicator;
}

/*
	error_add()

	Adds a new error to the error list.

	@warn	The name and format pointers must point to valid data as long
		as the errors may be emitted ! No data is copied.

	@arg	level	Error level.
	@arg	name	Error name. Error can be masked if the name begins with
			a '~' character.
	@arg	format	Error string format. Used through fprintf() using
			error_emit() arguments.
*/

void error_add(enum Error_Level level, const char *name, const char *format)
{
	// Allocating a new error type;
	struct Error *error = malloc(sizeof(struct Error));
	struct Error *parser = error_first;

	// If no error has been added already, setting error_first.
	if(!error_first) error_first = error;
	// Else, adding it at the end of the linked list.
	else
	{
		// Finding the last node.
		while(parser->next) parser = parser->next;
		// Linking the new error.
		parser->next = error;
	}


	// If the error name begins with '~' (ignored in options), the error
	// can be masked.
	if(*name == '~' && name[1])
	{
		// Skipping the '~' to keep only the interesting name.
		name++;
		// Setting the corresponding flag.
		error->maskable = 1;
	}
	// Otherwise, or if the name is just '~', resetting the flag.
	else error->maskable = 0;

	// Setting the error level.
	error->level = level;

	// Setting the error name and format string.
	error->name = name;
	error->format = format;

	// Activating the error by default.
	error->activated = 1;
	// Linking a NULL pointer at list end.
	error->next = NULL;
}

/*
	error_argument()

	Analyzes an argument string to disable errors. The argument must match
	the format '-[DNWE]<error_name>'. Fatal errors obviously cannot be
	disabled.

	@arg	argument	Argument string.

	@return		1 on error (unrecognized option), 0 otherwise.
*/

int error_argument(const char *argument)
{
	// Using an error level.
	enum Error_Level level = -1;
	// Using a return code.
	int ret = 1;
	// Using an error list parser.
	struct Error *parser = error_first;

	// Checking argument format : -[DNWE]<error_name>.
	if(*argument++ != '-') return 1;
	// Getting the error level.
	switch(*argument)
	{
		// Note and Debug levels are actually the same.
		case 'D': level = DEBUG; break;
		case 'N': level = NOTE; break;
		case 'W': level = WARNING; break;
		case 'E': level = ERROR; break;
		// Fatal errors, obviously,  cannot be disabled.
		default : return 1;
	}

	// Disable operation needs an error name !
	if(!*++argument) return 1;

	// Iterating over the errors.
	while(parser)
	{
		// Matching the maskable errors that match the given name.
		if(parser->level == level && !strcmp(parser->name, argument)
			&& parser->maskable)
		{
			// Disabling them.
			parser->activated = 0;
			// Setting the return value to 0.
			ret = 0;
		}

		// Getting to the next error.
		parser = parser->next;
	}

	// Returning ret, that is, 0 if any error has been disabled, 1
	// otherwise.
	return ret;
}

/*
	error_emit()

	Emits the error by printing a message on stderr, and possibly setting
	the failure indicator to 1 (if non-null) or exiting the program.

	@arg	level	Error level.
	@arg	name	Error name.
	@arg	...	Arguments to be given to the error format.
*/

void error_emit(enum Error_Level level, const char *name, ...)
{
	// Using strings to name the error types.
	const char *types[] = { "debug", "note", "warning", "error", "fatal "
		"error" };
	// Using a linked list parser.
	struct Error *parser = error_first;
	// Using an argument list.
	va_list args;

	// Starting the va_list to get the arguments for the format.
	va_start(args, name);

	// Parsing the defined error types to match the given one. It is
	// possible to use several error messages by having two errors with the
	// same name.
	while(parser)
	{
		// Testing if current one matches the given level and name.
		if(parser->level == level && !strcmp(parser->name, name)
			&& parser->activated)
		{
			// Writing the prefix.
			fputs(prefix, stderr);
			fputs(": ", stderr);

			// Writing the error type string representation.
			fputs(types[level], stderr);
			fputs(": ", stderr);

			// Writing the format string, formatted with the
			// function arguments.
			vfprintf(stderr, parser->format, args);
			// Adding a new line.
			fputc('\n', stderr);

			// On error, set the failure indicator if non-null.
			if(level == ERROR) if(failure) *failure = 1;
			// On fatal error, exiting the program.
			if(level == FATAL) exit(exit_code);
		}

		// Getting the next error.
		parser = parser->next;
	}

	// Ending the argument list.
	va_end(args);
}
