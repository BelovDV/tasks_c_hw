#include "text.h"
#include "check.h"
#include "utility.h"

#include <stdio.h>
#include <stdlib.h>

uint64_t text_get_division(Text_string_static text, char delimiter,
						   Text_string_index **dest)
{
	debug_check(dest != NULL);

	uint64_t length = text.length;

	uint64_t count = 0;
	for (uint64_t iter = 0; iter < text.length;)
	{
		while (iter < length && text.string[iter] == delimiter)
			++iter;
		while (iter < length && text.string[iter] != delimiter)
			++iter;
		if (iter < length || text.string[iter - 1] != delimiter)
			++count;
	}

	free(*dest);
	*dest = wrapped_calloc(count, sizeof(**dest));

	uint64_t position = 0;
	for (uint64_t iter = 0, last = 0; iter < length;)
	{
		while (last < length && text.string[last] == delimiter)
			++last;
		iter = last;
		while (iter < length && text.string[iter] != delimiter)
			++iter;
		if (iter > last)
		{
			(*dest)[position].index = last;
			(*dest)[position].length = iter - last;
			++position;
		}
		last = iter + 1;
	}
	debug_check(position == count);

	return count;
}

uint64_t text_get_division_with_empty(Text_string_static text, char delimiter,
									  Text_string_index **dest)
{
	debug_check(dest != NULL);
	size_t length = text.length;
	const char *string = text.string;

	size_t count = 1;
	for (size_t iter = 0; iter < length; ++iter)
		if (string[iter] == delimiter)
			++count;

	free(*dest);
	*dest = wrapped_calloc(count, sizeof(**dest));

	size_t position = 0;
	for (size_t number = 0; number < count; ++number)
	{
		(*dest)[number].index = position;
		while (position < length && string[position] != delimiter)
			++position;
		(*dest)[number].length = position - (*dest)[number].index;
		++position;
	}
	return count;
}

Text_string_static text_read_stream(FILE *stream)
{
	debug_check(stream != NULL);

	check(fseek(stream, 0, SEEK_END) == 0);
	uint64_t file_size = ftell(stream);
	check(fseek(stream, 0, SEEK_SET) == 0);

	char *text = (char *)malloc(file_size);
	check(text != NULL);

	check(fread(text, sizeof(*text), file_size, stream) == file_size);

	Text_string_static result;
	result.string = text;
	result.length = file_size;

	return result;
}

Text_string_static text_read_file(const char *filename)
{
	debug_check(filename != NULL);

	FILE *file = wrapped_fopen(filename, "r");

	Text_string_static result = text_read_stream(file);

	fclose(file);

	return result;
}

void text_write_stream(FILE *stream,
					   Text_string_static text,
					   uint64_t lines_count,
					   Text_string_index *lines,
					   char delimiter)
{
	debug_check(stream != NULL);
	debug_check(text.string != NULL);

	for (uint64_t i = 0; i < lines_count; ++i)
	{
		debug_check(lines[i].index + lines[i].length <= text.length);
		fprintf(stream, "%.*s%c",
				(int)(lines[i].length),
				text.string + lines[i].index,
				delimiter);
	}
}

void text_write_file(const char *filename, Text_string_static text,
					 uint64_t lines_count, Text_string_index *lines, char delimiter)
{
	debug_check(filename != NULL);
	debug_check(text.string != NULL);

	FILE *file = wrapped_fopen(filename, "w");

	text_write_stream(file, text, lines_count, lines, delimiter);
	fclose(file);
}