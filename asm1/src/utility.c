#include "utility.h"
#include "check.h"

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

void print_raw_data(FILE *stream, const void *data_, size_t size)
{
	debug_check(data_ != NULL);
	const uint8_t *data = data_;
	for (size_t i = 0; i < (size + 15) / 16; ++i)
		for (size_t j = 0; i * 16 + j < size; ++j)
			fprintf(stream, "%02hhx ", data[i * 16 + j]);
}

void print_raw_data_pretty(
	FILE *stream,
	const void *data_, size_t size,
	const char *prefix, int *start_address)
{
	debug_check(data_ != NULL);
	debug_check(prefix != NULL);
	const uint8_t *data = data_;
	if (prefix)
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

void utility_reserve(Array_frame *array, size_t new_cap)
{
	array->array = realloc(array->array, new_cap);
	check_message(array->array != NULL, check_calloc_failed, new_cap);
	array->capacity = new_cap;
}

FILE *utility_log = NULL;
size_t utility_const_log_level = 0;
void utility_log_close()
{
	fclose(utility_log);
}
