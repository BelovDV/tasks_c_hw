#include "check.h"

#include <stdio.h>
#include <stdlib.h>

void check_print_error_message(
	const char *file,
	int line,
	const char *function,
	const char *format,
	...)
{
	va_list argptr;
	va_start(argptr, format);
	fprintf(stderr,
			"ERROR: check failed:\n"
			"\tfile: %s:%d\n"
			"\tfunc: %s\n"
			"\t ",
			file, line, function);
	vfprintf(stderr,
			 format,
			 argptr);
	fprintf(stderr, "\n");
	va_end(argptr);
}

const char check_calloc_failed[] = "calloc failed size: %lu\n";
const char check_format_can_not_open_file[] = "can't open file \"%s\"\n";