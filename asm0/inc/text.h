#include <stdint.h>
#include <stdio.h>

#ifndef HEADER_TEXT
#define HEADER_TEXT

/**
 * @brief contains pointer to string and it's length
 */
typedef struct
{
	char *string;
	uint64_t length;
} Text_string_static;

/**
 * @brief contains index of string beggining in some text
 * 	and length of it
 */
typedef struct
{
	uint64_t index;
	uint64_t length;
} Text_string_index; // in other string

/**
 * @brief divide text by delimiter and write lines to dest (without empty lines)
 * @return count of lines
 * @exception free previous dest value
 */
uint64_t text_get_division(Text_string_static text, char delimiter,
						   Text_string_index **dest);

/**
 * @brief divide text by delimiter and write lines to dest
 * @return count of lines
 * @exception free previous dest value
 */
uint64_t text_get_division_with_empty(Text_string_static text, char delimiter,
									  Text_string_index **dest);

Text_string_static text_read_stream(FILE *stream);
Text_string_static text_read_file(const char *filename);

/**
 * @brief write text to stream divide lines by delimiter
 */
void text_write_stream(FILE *stream, Text_string_static text,
					   uint64_t lines_count, Text_string_index *lines, char delimiter);

/**
 * @brief write text to file divide lines by delimiter
 */
void text_write_file(const char *filename, Text_string_static text,
					 uint64_t lines_count, Text_string_index *lines, char delimiter);

typedef struct
{
	uint32_t count;
	char **strings; // array of array of char
} Text;

#endif