#include <stdio.h>
#include <stdlib.h>

#ifndef HEADER_UTILITY
#define HEADER_UTILITY

/**
 * @brief check if calloc returns non-zero pointer
 * 
 * @exception ONLY IN EXPRESSIONS LIKE int* ptr = wrapped_calloc(ptr, 1);
 * 
 * @note This weird syntax is bad, but it allows don't forgot sizeof
 */
#define wrapped_calloc(ptr, number)       \
	ptr = calloc(number, sizeof(*(ptr))); \
	check_message(ptr != NULL, check_calloc_failed, number * sizeof(*ptr))

/**
 * @brief check if fopen returns non-zero pointer
 * 
 * @exception ONLY IN EXPRESSIONS LIKE FILE* ptr = wrapped_fopen(ptr, "input.txt", "r");
 * 
 * @note This weird syntax is bad, but it allows don't forgot sizeof
 */
#define wrapped_fopen(ptr, filename, mods) \
	ptr = fopen(filename, mods);           \
	check_message(ptr != NULL, check_format_can_not_open_file, filename)

/**
 * @brief hash your data[0; size-1]
 */
size_t hash(const void *data, size_t size);

/**
 * @brief count amount of symbol in str
 */
size_t strcnt(const char *str, char symbol);

/**
 * @brief print size bytes from data to stream with format: "XX "
 * 
 * @param line size of line, 0 if don't use several lines
 */
void print_raw_data(FILE *stream, const void *data, size_t size, long line);

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
 * @brief reallocation of array (with check calloc)
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

#ifdef NDEBUG
#define LOG(var, type)
#define LOG_L(var, type)
#define LOG_LL(var, vsp, type)
#define LOG_I
#define LOG_D
#else
extern FILE *utility_log;
void utility_log_close();
extern size_t utility_const_log_level;
#ifndef UTILITY_LOG_FILE
#define UTILITY_LOG_FILE "log_utility"
#endif
#define UTILITY_FOPEN                                      \
	if (utility_log == NULL)                               \
	{                                                      \
		wrapped_fopen(utility_log, UTILITY_LOG_FILE, "w"); \
		setvbuf(utility_log, NULL, _IONBF, 0);             \
		atexit(utility_log_close);                         \
	}
#define LOG_PREFACE \
	UTILITY_FOPEN   \
	fprintf(utility_log, "%-20.20s %d  \t", __FILE__, __LINE__);
#define LOG(var, type)                                                      \
	fprintf(utility_log,                                                    \
			"LOG:\n\tfile %s\n\tfunc %s\n\tline %3d:\n\t'%s' = " type "\n", \
			__FILE__, __PRETTY_FUNCTION__, __LINE__, #var, var);
#define LOG_L(var, type)                                           \
	{                                                              \
		LOG_PREFACE                                                \
		for (size_t i = 0; i < utility_const_log_level; ++i)       \
			fprintf(utility_log, "    ");                          \
		fprintf(utility_log, "'%.30s' = " type "\n", #var, (var)); \
	}
#define LOG_LL(var, vsp, type)                                         \
	{                                                                  \
		LOG_PREFACE                                                    \
		for (size_t i = 0; i < utility_const_log_level; ++i)           \
			fprintf(utility_log, "    ");                              \
		fprintf(utility_log, "'%s' = " type "\n", #var, (vsp), (var)); \
	}
#define LOG_I ++utility_const_log_level;
#define LOG_D --utility_const_log_level;
#endif

#endif