#include <stdio.h>
#include <stdlib.h>

#ifndef HEADER_UTILITY
#define HEADER_UTILITY

/**
 * @brief hash your data[0; size-1]
 */
size_t hash(const void *data, size_t size);

/**
 * @brief count amount of symbol in str
 */
size_t strcnt(const char *str, char symbol);

/**
 * @brief '00 11 22 33 44 55 66 77 88 99 aa bb ...'
 */
void print_raw_data(FILE *stream, const void *data, size_t size);

/**
 * @brief print data in raw appearance to stream
 * 
 * @param prefix will be written at the begining of all lines if nonzero
 * otherwise there will be one line
 * @param start_address there will be written address of first byte of each line
 * if prefix and start_address are nonzero
 */
void print_raw_data_pretty(
	FILE *stream,
	const void *data, size_t size,
	const char *prefix, int *start_address);

/**
 * @brief frame for array...
 */
typedef struct
{
	void *array;
	size_t size;
	size_t capacity;
} Array_frame;

/**
 * @brief reallocation of array
 */
void utility_reserve(Array_frame *array, size_t new_cap);

/**
 * @brief frame for static array...
 */
typedef struct
{
	void *array;
	size_t size;
} Array_static_frame;

#endif