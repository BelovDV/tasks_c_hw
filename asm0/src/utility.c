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

void print_raw_data(FILE *stream, const void *data_, size_t size, long line)
{
	const uint8_t *begin = data_;
	const uint8_t *end = begin + size;
	const uint8_t *vsp = begin;
	if (line > 0)
		fprintf(stream, "\n0x%-6lx", 0lu);
	for (const uint8_t *iter = begin; iter < end; ++iter)
	{
		if (line > 0 && ((iter - vsp) == line))
		{
			fprintf(stream, "\n0x%-6lx", iter - begin);
			vsp = iter;
		}
		fprintf(stream, "%.2x ", (int)*(iter));
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
