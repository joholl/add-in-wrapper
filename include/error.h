/*
**  A module that handles error and warning management for
**  command-line applications.
*/

#ifndef _ERROR_H
#define _ERROR_H

// Error level enumeration.
	enum ErrorLevel
	{
		NOTE    = 1,
		WARNING = 2,
		ERROR   = 4,
		FATAL   = 8
	};

// Error type structure (linked list).
	struct ErrorType
	{
		int level;
		const char *name;
		const char *format;
		int activated :4;
		int maskable  :4;

		struct ErrorType *next;
	};

// Functions declaration.
	void error_init(const char *programName, int defaultExitCode, int *failureIndicator);
	void error_add(int level, const char *name, const char *format);
	int  error_argument(const char *argument);
	void error_emit(int level, const char *name, ...);
	void error_static(int x, const char *programName, int defaultExitCode, int *failureIndicator);

#endif
