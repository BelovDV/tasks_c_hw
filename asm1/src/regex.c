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

static void printer_char(void *stream, void *symbol)
{
	LOG("%p", symbol)
	print_char(stream, symbol);
}
#define IN                     \
	LOG_IN                     \
	LOG_F(printer_char, RULE); \
	LOG_F(printer_char, ITER); \
	int is_error = 0;
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
static int brackets(Regex *regex);	 // ()
static int range(Regex *regex);		 // []
static int quest(Regex *regex);		 // ?
static int escape(Regex *regex);	 // $
static int symbol(Regex *regex);	 // other

int regex(Regex *regex)
{
	LOG_IN
	LOG("%p", RULE)
	LOG("%p", ITER)
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
	LOG("%p", RULE)
	while (1)
	{
		if (*RULE == '\0' || *RULE == ')')
			break;
		if (*RULE == '|')
		{
			if (is_error)
				is_error = 0;
			else
				FLAG |= is_true;
			++RULE;
		}
		if (is_error)
			break;
		is_error = expression(regex);
	}
	OUT
}

static int expression(Regex *regex)
{
	IN;
	CHECK(*ITER != '\0')
	CHECK(*RULE != '\0')
	if (*RULE == '(')
		is_error = brackets(regex);
	else if (*RULE == '?')
		is_error = quest(regex);
	else if (*RULE == '[')
		is_error = range(regex);
	else if (*ITER == '$')
		is_error = escape(regex);
	else
		is_error = symbol(regex);
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
	IN;
	// CHECK(RULE[0] == '[')
	CHECK(RULE[1] != '\0' && RULE[2] != '\0' && RULE[1] >= RULE[2])
	CHECK(RULE[3] == ']')
	is_error = RULE[1] <= *ITER && *ITER <= RULE[2];
	RULE += 4;
	ITER += 1;
	OUT
}

static int quest(Regex *regex)
{
	IN;
	CHECK(0)
	OUT
}

static int escape(Regex *regex)
{
	IN;
	++RULE;
	is_error = symbol(regex);
	OUT
}

static int symbol(Regex *regex)
{
	IN;
	is_error = (*RULE == *ITER);
	++RULE;
	++ITER;
	OUT
}