#ifndef HEADER_CHECK
#define HEADER_CHECK

#define CHECK_EXIT_VALUE 2

void check_print_error_message(
	const char *file,
	int line,
	const char *function,
	const char *format,
	...);

#define check(condition)               \
	do                                 \
	{                                  \
		if (!(condition))              \
		{                              \
			check_print_error_message( \
				__FILE__,              \
				__LINE__,              \
				__PRETTY_FUNCTION__,   \
				#condition);           \
			exit(CHECK_EXIT_VALUE);    \
		}                              \
	} while (0)

#define check_message(condition, format, argptr...) \
	do                                              \
	{                                               \
		if (!(condition))                           \
		{                                           \
			check_print_error_message(              \
				__FILE__,                           \
				__LINE__,                           \
				__PRETTY_FUNCTION__,                \
				format,                             \
				argptr);                            \
			exit(CHECK_EXIT_VALUE);                 \
		}                                           \
	} while (0)

extern const char check_calloc_failed[];
extern const char check_format_can_not_open_file[];

#ifndef NDEBUG
#define debug_check(condition) check(condition)
#else
#define debug_check(condition)
#endif

#endif