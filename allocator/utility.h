#include <stdio.h>
#include <stdlib.h>

#ifndef HEADER_UTILITY
#define HEADER_UTILITY

/**
 * @brief check if calloc returns non-zero pointer
 */
void *wrapped_calloc(size_t number, size_t size);

/**
 * @brief check if fopen returns non-zero pointer
 */
FILE *wrapped_fopen(const char *filename, const char *modes);

/**
 * @brief hash your data[0; size-1]
 */
size_t hash(const void *data, size_t size);

/**
 * @brief frame for array...
 */
struct Array_frame
{
	size_t size;
	size_t capacity;
	void *array;
};

#endif