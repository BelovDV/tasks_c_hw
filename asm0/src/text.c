#include "text.h"
#include "check.h"
#include "utility.h"

#include <errno.h>
#include <string.h>

#ifdef FALL_ON_ERROR
#define ERRNO check(errno == 0);
#define ALLOC(ptr, size)              \
	ptr = calloc(size, sizeof(*ptr)); \
	check_message(ptr != NULL, check_calloc_failed, size);
#define FOPEN(ptr, name, parameters)     \
	FILE *ptr = fopen(name, parameters); \
	check_message(ptr != NULL, check_format_can_not_open_file, name);
#else
#define END_IF(error)  \
	if (error)         \
	{                  \
		return result; \
	}
#define ERRNO \
	END_IF(errno)
#define ALLOC(ptr, size)              \
	ptr = calloc(size, sizeof(*ptr)); \
	END_IF(ptr == NULL)
#define FOPEN(ptr, name, parameters)     \
	FILE *ptr = fopen(name, parameters); \
	END_IF(ptr == NULL)
#endif

Text_string text_read_stream(FILE *stream)
{
	debug_check(stream != NULL);
	errno = 0;

	Text_string result = {0, 0};

	fseek(stream, 0, SEEK_END);
	result.size = ftell(stream);
	fseek(stream, 0, SEEK_SET);
	ERRNO

	ALLOC(result.value, result.size)
	fread(result.value, sizeof(*result.value), result.size, stream);
	ERRNO

	return result;
}

Text_string text_read_file(const char *filename)
{
	debug_check(filename != NULL);

	Text_string result = {0, 0};

	FOPEN(file, filename, "r")
	result = text_read_stream(file);
	fclose(file);

	return result;
}

int text_write_stream(FILE *stream, Text text, const char *del)
{
	debug_check(stream != NULL);
	debug_check(text.size == 0 || text.indexation != NULL);

	for (size_t i = 0; i < text.size; ++i)
		if (fprintf(stream, "%.*s%s",
					(int)text.indexation[i].size,
					text.indexation[i].value,
					del) < 0)
			return 1;
	return 0;
}

int text_write_file(const char *filename, Text text, const char *del)
{
	debug_check(filename != NULL);
	debug_check(text.size == 0 || text.indexation != NULL);

	int result = 1;

	FOPEN(file, filename, "w")

	result = text_write_stream(file, text, del);

	fclose(file);

	return result;
}

#define IS_DEL(symbol) strcnt(delimiters, symbol)

Text text_decompose(Text_string text, const char *delimiters, int skip_empty)
{
	debug_check(text.size == 0 || text.value != NULL);

	Text result = {0, 1};
	size_t length = text.size;
	char *string = text.value;

	size_t previous_del = 1; // is previous symbol delimiter (bool)
	for (size_t iter = 0; iter < length; ++iter)
		if (IS_DEL(string[iter]) && !(skip_empty && previous_del))
			++result.size, previous_del = 1;
		else
			previous_del = 0;

	ALLOC(result.indexation, result.size)
	size_t start = 0; // position after previous delimiter (size_t)
	size_t line = 0;  // number of line
	for (size_t iter = 0; iter < length; ++iter)
		if (IS_DEL(string[iter]) && (!skip_empty || start < iter))
		{
			result.indexation[line].size = iter - start;
			result.indexation[line].value = string + start;
			start = iter + 1;
			++line;
		}

	if (!skip_empty || start < length)
	{
		result.indexation[line].size = length - start;
		result.indexation[line].value = string + start;
	}
	debug_check(line + 1 == result.size);
	return result;
}