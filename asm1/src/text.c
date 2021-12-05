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
	debug_check(text.size == 0 || text.index != NULL);

	for (size_t i = 0; i < text.size; ++i)
		if (fprintf(stream, "%.*s%s",
					(int)text.index[i].size, // int is too little...
					text.index[i].value,
					del) < 0)
			return 1;
	return 0;
}

int text_write_file(const char *filename, Text text, const char *del)
{
	debug_check(filename != NULL);
	debug_check(text.size == 0 || text.index != NULL);

	int result = 1;

	FOPEN(file, filename, "w")

	result = text_write_stream(file, text, del);

	fclose(file);

	return result;
}

int text_write_raw_file(const char *filename, Text_string *data)
{
	debug_check(filename != NULL);
	debug_check(data->size == 0 || data->value != NULL);

	int result = 1;
	FOPEN(file, filename, "w")

	result = !(fwrite(data->value, data->size, 1, file) > 0);
	fclose(file);

	return result;
}

#define IS_DEL(symbol) strcnt(delimiters, symbol)

Text text_decompose(const Text_string *text,
					const char *delimiters, int skip_empty)
{
	debug_check(text->size == 0 || text->value != NULL);

	Text result = {0, 0};

	size_t length = text->size;
	char *string = text->value;
	// printf("skip empty %d\n", skip_empty);
	// printf("length %lu\n", length);

	int is_after_del = 1;
	for (size_t iter = 0; iter < length; ++iter)
		if (IS_DEL(string[iter]))
		{
			if (!is_after_del || !skip_empty)
				result.size += 1;
			is_after_del = 1;
		}
		else
			is_after_del = 0;
	if (!is_after_del)
		result.size += 1;

	// printf("result.size %lu\n", result.size);

	ALLOC(result.index, result.size)
	size_t start = 0; // position after previous delimiter (size_t)
	size_t line = 0;  // number of line
	for (size_t iter = 0; iter < length; ++iter)
	{
		// printf("%lu\t", iter);
		// printf("%d\t", (int)string[iter]);
		if (IS_DEL(string[iter]))
		{
			if (!is_after_del || !skip_empty)
			{
				result.index[line].size = iter - start;
				result.index[line].value = string + start;
				++line;
				// printf("\tadded");
			}
			is_after_del = 1;
			start = iter + 1;
		}
		else
			is_after_del = 0;
		// printf("\tis after %d\t", is_after_del);
		// printf("line %lu\n", line);
	}
	if (!is_after_del)
	{
		result.index[line].size = length - start;
		result.index[line].value = string + start;
		start = length + 1;
		++line;
		// printf("\tadded");
		// printf("\tis after %d\t", is_after_del);
		// printf("line %lu\n", line);
	}

	// printf("\nline %lu\n", line);

	debug_check(line == result.size);

	for (size_t i = 0; i < result.size; ++i)
	{
		// printf("'%.*s'\n", (int)result.index[i].size, result.index[i].value);
	}

	// printf("done\n\n\n");

	return result;
}