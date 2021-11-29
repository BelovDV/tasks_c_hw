#include <stdio.h>
#include <stdlib.h>

#ifndef HEADER_TEXT
#define HEADER_TEXT

/**
 * @brief contains pointer to string and it's length
 */
typedef struct
{
	char *value;
	size_t size;
} Text_string;

/**
 * @brief contains array of string_static - part of some string
 */
typedef struct
{
	Text_string *indexation;
	size_t size;
} Text;

/**
 * @brief read up to eof stream, add 0 to end (string[length])
 */
Text_string text_read_stream(FILE *stream);

/**
 * @brief read up to eof file, add 0 to end (string[length])
 */
Text_string text_read_file(const char *filename);

/**
 * @brief write text decomposition to stream with del between lines
 * 
 * @exception delimiter will ends file
 * 
 * @return error (correct - 0)
 */
int text_write_stream(FILE *stream, Text text, const char *delimiter);

/**
 * @brief write text decomposition to file with del between lines
 * 
 * @exception delimiter will ends file
 * 
 * @return error (correct - 0)
 */
int text_write_file(const char *filename, Text text, const char *delimiter);

/**
 * @brief make decomposition of text dividing it by delimiters
 * @exception in index string[length] - delimiter
 */
Text text_decompose(Text_string text, const char *delimiters, int skip_empty);

#endif