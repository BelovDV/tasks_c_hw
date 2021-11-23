#include "utility.h"
#include "check.h"

#include <stdint.h>

void *wrapped_calloc(size_t number, size_t size)
{
	// how to check number*size < 1<<64 ?
	void *result = calloc(number, size);
	check_message(result != NULL, check_calloc_failed, number * size);
	return result;
}

FILE *wrapped_fopen(const char *filename, const char *modes)
{
	FILE *result = fopen(filename, modes);
	check_message(result != NULL, check_format_can_not_open_file, filename);
	return result;
}

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