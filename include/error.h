/*
	Error module.

	A simple module to handle error and warning management in command-line
	applications.
*/

#ifndef _ERROR_H
	#define _ERROR_H 1

/*
	Composed types definitions.
*/

// Error level enumeration.
enum Error_Level
{
	DEBUG   = 0,
	NOTE    = 1,
	WARNING = 2,
	ERROR   = 3,
	FATAL   = 4
};



/*
	Function prototypes.
*/

// Initializing the module.
void error_init(const char *prefix, int exit_code, int *failure);
// Adding errors.
void error_add(enum Error_Level level, const char *name, const char *format);
// Disabling error with a command-line argument of format -[DNWE]<error_name>.
int  error_argument(const char *argument);
// Emitting errors on stderr.
void error_emit(enum Error_Level level, const char *name, ...);

#endif // _ERROR_H
