#include "regex.h"
#include "log.h"
#include "utility.h"

#ifndef HEADER_REGEX // code doesnot see regex.h - it is blocked by posix regex
#define HEADER_REGEX

typedef struct
{
	char *rule;				// pointer to main rule
	char **additional_rule; // pointer to set of rules
	char **result;			// pointer to dest of '?', should be allocated
	char *considered;		// string which one to process

	int flags; // inner
} Regex;

#endif

#define ITER (regex->considered)
#define RULE (regex->rule)
#define FLAG (regex->flags)

#define CHECK(condition)                 \
	{                                    \
		if (!(condition))                \
		{                                \
			FLAG |= is_syntax_error;     \
			LOG(#condition "failed", "") \
			return 1;                    \
		}                                \
	}

#ifdef SHUTDOWN_REGEX
#undef LOG
#undef LOG_IN
#undef LOG_OUT
#undef LOG_F
#define LOG_IN
#define LOG_OUT
#define LOG(format, var)
#define LOG_F(func, var)
#else
static void printer_char(void *stream, void *symbol)
{
	print_char(stream, symbol);
}
#endif

#define IN               \
	LOG_IN               \
	LOG("'%.10s'", RULE) \
	LOG("'%.10s'", ITER) \
	int is_error = 0;
// LOG_F(printer_char, *RULE);
// LOG_F(printer_char, *ITER);

#define OUT             \
	LOG_OUT             \
	LOG("%d", is_error) \
	return is_error;

enum
{
	is_true = 0x01,
	is_syntax_error = 0x02,
}; // TODO - global flag for comparing prefix of str or with prefix of rule

static int line(Regex *regex);		 // binary operators: '|'
static int expression(Regex *regex); // choose single
static int repeat(Regex *regex);	 // *
static int brackets(Regex *regex);	 // ()
static int range(Regex *regex);		 // []
static int quest(Regex *regex);		 // ?
static int symbol(Regex *regex);	 // symbols (or after $)

int regex(Regex *regex)
{
	LOG_IN
	LOG("%p", RULE)
	LOG("%p", ITER)
	LOG("%p", regex->result)
	LOG("%s", RULE)
	LOG("%s", ITER)
	regex->flags = 0;
	int is_error = line(regex);
	LOG_OUT
	return *ITER || *RULE || is_error || (FLAG & is_syntax_error);
}

static int line(Regex *regex)
{
	IN;
	char *start = ITER;
	char *after_first_true = NULL;
	while (1)
	{
		char *expression_start = ITER;
		while (*RULE != '|' && *RULE != ')' && *RULE != '\0')
			is_error |= expression(regex);
		if (!is_error && !after_first_true)
			after_first_true = ITER, FLAG |= is_true;
		else
			ITER = expression_start;
		if (*RULE != '|')
			break;
		if (is_error)
			is_error = 0;
		++RULE;
		ITER = start;
		LOG("%d", (int)(after_first_true != NULL))
	}
	FLAG &= ~is_true;
	if (after_first_true)
		ITER = after_first_true, is_error = 0;
	OUT
}

static int expression(Regex *regex)
{
	IN;
	// LOG("%p", RULE)
	// LOG("%p", ITER)
	if (*RULE == '?' || *RULE == '!')
		is_error = quest(regex);
	else if (*RULE == '*')
		is_error = repeat(regex);
	else if (*RULE == '(')
		is_error = brackets(regex);
	else if (*RULE == '[')
		is_error = range(regex);
	else if (*RULE == ']' || *RULE == ')')
		is_error = 1;
	else if (*RULE == '$')
		++RULE, is_error = symbol(regex);
	else
		is_error = symbol(regex);
	OUT
}

static int repeat(Regex *regex)
{
	IN;
	// if (*RULE != '*') return 1;
	++RULE;
	char *expr_start = ITER;
	char *rule_start = RULE;
	while (!expression(regex))
		expr_start = ITER, RULE = rule_start;
	ITER = expr_start; // return to first wrong
	OUT
}

static int brackets(Regex *regex)
{
	// if (*RULE != '(') return 1;
	IN;
	++RULE;
	is_error = line(regex);
	CHECK(*RULE == ')')
	++RULE;
	OUT
}

static int range(Regex *regex)
{
	// CHECK(RULE[0] == '[')
	IN;
	// LOG_F(printer_char, RULE[0])
	// LOG_F(printer_char, RULE[1])
	// LOG_F(printer_char, RULE[2])
	// LOG_F(printer_char, RULE[3])
	// LOG_F(printer_char, RULE[4])
	LOG_F(printer_char, *ITER)
	CHECK(RULE[1] != '\0' && RULE[2] != '\0' && RULE[3] != '\0')
	CHECK(RULE[1] < RULE[3] && RULE[2] == '-')
	CHECK(RULE[4] == ']')
	is_error = RULE[1] > *ITER || *ITER > RULE[3];
	RULE += 5;
	ITER += 1;
	OUT
}

static int quest(Regex *regex)
{
	IN;
	int require = *RULE == '!';
	char *start = ITER;
	++RULE;
	char **result = regex->result;
	regex->result += 1;
	is_error = expression(regex);
	// LOG("%p", regex->result)
	// LOG("%p", *regex->result)
	////LOG("%d", FLAG & is_true)
	if ((FLAG & is_true) || is_error)
		*(result) = NULL;
	else
		*(result) = start;
	// LOG("%p", regex->result)
	LOG("%p", *result)
	if (!require)
		is_error = 0;
	OUT
}

static int symbol(Regex *regex)
{
	IN;
	if (*RULE == *ITER)
		++ITER;
	else
		is_error = 1;
	++RULE;
	OUT
}