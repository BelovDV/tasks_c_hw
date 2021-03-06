/*
	it isn't ready yet - just searching
*/

#include <stdio.h>
#include <stdlib.h>

#ifndef HEADER_UTILITY
#define HEADER_UTILITY

/**
 * @brief hash your data[0; size-1]
 */
size_t hash(const void *data, size_t size);

/**
 * @brief count amount of symbol in str (null terminated)
 */
size_t strcnt(const char *str, char symbol);

/**
 * @brief count amount of symbol in str (with length)
 */
size_t strncnt(const char *str, char symbol, size_t length);

/**
 * @brief length of str (null terminated) up to one of delimiters
 */
size_t strlen_del(const char *str, const char *delimiters);

/**
 * @brief length of str (with length) up to one of delimiters
 */
size_t strnlen_del(const char *str, const char *delimiters, size_t length);

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
 * @brief reallocation of Array_frame
 * @return was_error
 * @exception size and capacity NOT in bytes, but in elements
 */
int utility_array_realloc(Array_frame *array, size_t new_cap, size_t el_sz);

/**
 * @brief reserve new memory for Array_frame if necessary
 * @return was_error
 * @exception size and capacity NOT in bytes, but in elements
 */
int utility_array_provide_space(Array_frame *array,
								size_t element_size,
								size_t required_space);

/**
 * @brief a b c 1 2 _ ? '\n' '\t' ...
 * @param symbol pointer to symbol which to print
 * @return 0 - wasn't configured; 1 - '\.'; 2 - as integer
 * @exception asci only
 */
int print_char(FILE *stream, char *symbol);

#endif