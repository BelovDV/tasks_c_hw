#include "log.h"
#include "check.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef EXTRA_LOG

#define LOG_MAX_STRING_NUMBER 4096

static int depth = 0;
static FILE *log_file = NULL;

static int initialize = 1;
static int string_number = 0;

#ifdef LOG_MAX_STRING_NUMBER
#define CHECK_NUMBER                            \
	if (string_number >= LOG_MAX_STRING_NUMBER) \
	{                                           \
		if (log_file)                           \
		{                                       \
			fclose(log_file);                   \
			log_file = NULL;                    \
		}                                       \
		return;                                 \
	}                                           \
	++string_number;
#else
#define CHECK_NUMBER
#endif

void log_in()
{
	++depth;
}

void log_out()
{
	--depth;
}

static void close()
{
	if (log_file)
		fclose(log_file);
}

static void init()
{
	log_file = fopen(LOG_FILE, "a");
	check_message(log_file != NULL, check_format_can_not_open_file, LOG_FILE);
	setvbuf(log_file, NULL, _IONBF, 0);
	atexit(close);
}

void indent()
{
	for (int len = 0; len < depth; ++len)
		if (len % 4 == 0)
			fprintf(log_file, "|   ");
		else if (len % 2 == 0)
			fprintf(log_file, ":   ");
		else
			fprintf(log_file, ".   ");
}

void log_message(const char *file, int line, const char *format, ...)
{
	CHECK_NUMBER
	debug_check(file != NULL && format != NULL);
	if (initialize)
	{
		initialize = 0;
		init();
	}

	va_list argptr;
	va_start(argptr, format);
	int len;
	fprintf(log_file, "%s:%d: %n", file, line, &len);
	while (len++ < 25)
		fprintf(log_file, " ");
	indent();
	vfprintf(log_file, format, argptr);
	fprintf(log_file, "\n");

	va_end(argptr);
}

void log_func(const char *file, int line,
			  const char *name,
			  void (*printer)(void *, void *), void *var)
{
	CHECK_NUMBER
	debug_check(file != NULL && name != NULL && printer != NULL && var != NULL);
	if (initialize)
	{
		initialize = 0;
		init();
	}

	int len;
	fprintf(log_file, "%s:%d: %n", file, line, &len);
	while (len++ < 25)
		fprintf(log_file, " ");
	indent();
	fprintf(log_file, "'%-20.20s' = ", name);
	printer(log_file, var);
	fprintf(log_file, "\n");
}

#endif