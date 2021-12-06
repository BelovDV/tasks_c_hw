#include "utility.h"

#include <stdint.h>

size_t hash(const void *data_, size_t size)
{
	const uint8_t *data = data_;
	size_t result = 123;
	++size;
	while (size-- != 0)
	{
		result = (result << 2) + (size_t)data[size];
		result ^= result >> 5;
	}
	return result;
}

size_t strcnt(const char *str, char symbol)
{
	size_t result = 0;
	while (*str != '\0')
		if (*(str++) == symbol)
			++result;
	return result;
}

size_t strncnt(const char *str, char symbol, size_t length)
{
	size_t count = 0;
	while (length--)
		if (*(str++) == symbol)
			++count;
	return count;
}

size_t strlen_del(const char *str, const char *delimiters)
{
	debug_check(str != NULL && delimiters != NULL);
	for (size_t i = 0;; ++i)
		if (str[i] == '\0' || strcnt(delimiters, str[i]))
			return i;
}

size_t strnlen_del(const char *str, const char *delimiters, size_t length)
{
	debug_check(str != NULL && delimiters != NULL);
	for (size_t i = 0; i < length; ++i)
		if (strcnt(delimiters, str[i]))
			return i;
	return length;
}

void print_raw_data(FILE *stream, const void *data_, size_t size)
{
	debug_check(data_ != NULL);
	const uint8_t *data = data_;
	for (size_t i = 0; i < size; ++i)
		fprintf(stream, "%02hhx ", data[i]);
}

void print_raw_data_pretty(
	FILE *stream,
	const void *data_, size_t size,
	const char *prefix, int *start_address)
{
	debug_check(data_ != NULL);
	debug_check(prefix != NULL);
	const uint8_t *data = data_;
	for (size_t i = 0; i < (size + 15) / 16; ++i)
	{
		fprintf(stream, "%s", prefix);
		if (start_address)
			fprintf(stream, "0x%04x:\t", *start_address + (int)i * 16);
		for (size_t j = 0; i * 16 + j < size; ++j)
			fprintf(stream, "%02hhx ", data[i * 16 + j]);
		fprintf(stream, "\n");
	}
}

int utility_array_realloc(Array_frame *array, size_t new_cap, size_t el_sz)
{
	void *new_array = realloc(array->array, new_cap * el_sz);
	if (!new_array)
		return 1;
	array->array = new_array;
	array->capacity = new_cap;
	return 0;
}

int utility_array_provide_space(Array_frame *array,
								size_t element_size,
								size_t required_space)
{
	if (array->size + required_space > array->capacity)
		return utility_array_realloc(
			array,
			(array->capacity > required_space)
				? array->capacity * 2
				: required_space,
			element_size);
	return 0;
}

int print_char(FILE *stream, char *symbol_)
{
	unsigned char *symbol = (unsigned char *)symbol_;
	if (*symbol >= 33 && *symbol <= 126)
	{
		fprintf(stream, "%c", *symbol);
		return 0;
	}
	if (*symbol == '\n')
	{
		fprintf(stream, "'\\n'");
		return 1;
	}
	if (*symbol == '\t')
	{
		fprintf(stream, "'\\t'");
		return 1;
	}
	if (*symbol == ' ')
	{
		fprintf(stream, "' '");
		return 1;
	}
	if (*symbol == '\0')
	{

		fprintf(stream, "'\\0'");
		return 1;
	}
	fprintf(stream, "%d", (int)*symbol);
	return 2;
}