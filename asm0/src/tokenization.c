#include "tokenization.h"
#include "language.h"
#include "check.h"

#include <stdlib.h>
#include <string.h>

const char token_errors[e_tokenization_error_count][64] =
	{
		"nothing wrong",
		"wrong line",
		"unsupported",
		"deprecated",
		"wrong section",
		"too long line",
		"multiply start"};

void tokenization_initialise(struct Tokenization_condition *condition)
{
	debug_check(condition != NULL);
	condition->dictionary = NULL;
	condition->flags = 0;
	condition->last_instr.id = e_instr_id_nothing;
	condition->section = e_token_section_nothing;
}

void static token_decode_instr(
	struct Tokenization_condition *condition,
	const char *begin, const char *end, size_t line);

void static token_decode_data(
	struct Tokenization_condition *condition,
	const char *begin, const char *end, size_t line);

void tokenization_decode(struct Tokenization_condition *condition,
						 const char *begin, const char *end, size_t line)
{
	debug_check(condition != NULL);
	debug_check(begin != NULL);
	debug_check(end != NULL);
	debug_check(begin <= end);

	printf("Line %lu:\n\t\"", line);
	for (const char *iter = begin; iter != end; ++iter)
	{
		printf("%c", *iter);
	}
	printf("\"\n");

	if (end == begin)
		return;

	if (end - begin >= e_const_tokenization_max_line_length)
	{
		condition->error_id = e_tokenization_error_too_long_line;
		return;
	}

	condition->section = e_token_section_text; // TODO!!!

	condition->flags &= ~e_token_flag_mask_data_changed;
	condition->flags &= ~e_token_flag_mask_intstr_changed;
	switch (*begin)
	{
	case ':': // label
	case '.': // section
		condition->error_id = e_tokenization_error_unsupported;
		break;
	case '_': // start
		if (condition->flags & e_token_flag_mask_start)
			condition->error_id = e_tokenization_error_multiply_start;
		condition->flags |= e_token_flag_mask_start;
		break;

	case '\t': // instruction / data
		if (condition->section == e_token_section_text)
			token_decode_instr(condition, begin, end, line);
		else if (condition->section == e_token_section_data)
			condition->error_id = e_tokenization_error_unsupported;
		else
			condition->error_id = e_tokenization_error_wrong_section;
		break;

	default:
		condition->error_id = e_tokenization_error_wrong_line;
		break;
	}
}

/**
 * @brief find string in patterns[]
 * @return id of found (or -1)
 */
int static token_find_keyword(
	const char patterns[][e_const_language_max_word_length],
	int max_id, const char *string)
{
	debug_check(patterns != NULL);
	debug_check(max_id > 0);
	for (int id = 0; id < max_id; ++id)
	{
		if (strcmp(patterns[id], string) == 0)
			return id;
	}
	return -1;
}

int static token_parse_arg(const char *name, struct Argument *result)
{
	size_t length = strlen(name);
	if (name[0] == '[')
	{
		if (name[length - 1] != ']')
			return 1;
		++name;
		length -= 2;
		result->is_memory = 1;
	}
	int rx, ry, k;
	uint64_t c;
	if (sscanf(name, "r%d+%d*r%d+%lu", &rx, &k, &ry, &c) > 0)
	{
		result->constant = c;
		result->k = (uint8_t)k;
		result->rx = (uint8_t)rx;
		result->ry = (uint8_t)ry;
		return 0;
	}
	if (sscanf(name, "%lu", &c) > 0)
	{
		result->constant = c;
		result->k = 0;
		result->rx = 15;
		result->ry = 15;
		return 0;
	}
	return 1;
}

void static token_decode_instr(
	struct Tokenization_condition *condition,
	const char *begin, const char *end, size_t line)
{
	char buffer[e_const_tokenization_max_line_length + 1];
	char word[5][e_const_tokenization_max_line_length + 1];

	memcpy(buffer, begin, end - begin);
	buffer[end - begin] = '\0';

	size_t word_count = 0;
	int result;
	if ((result = sscanf(buffer, "%s%s%s%s%s",
						 word[0], word[1], word[2], word[3], word[4])) > 0)
		word_count += result;

	if (result == -1)
		return;
	debug_check(result != 0);

	for (size_t iter = 0; iter < word_count; ++iter)
		printf("\t\t\"%s\"\n", word[iter]);

	int id = token_find_keyword(instruction_names, e_instr_id_count, word[0]);
	printf("   id: %d\n", id);

	if (id == -1 || word_count > 4)
	{
		condition->error_id = e_tokenization_error_wrong_line;
		return;
	}

	condition->flags |= e_token_flag_mask_intstr_changed;

	condition->last_instr.id = id;
	condition->last_instr.argc = word_count - 1;
	for (size_t argn = 1; argn < word_count; ++argn)
	{
		if (token_parse_arg(word[argn], &condition->last_instr.argv[argn - 1]))
			condition->error_id = e_tokenization_error_wrong_line;
	}
}

void static token_decode_data(
	struct Tokenization_condition *condition,
	const char *begin, const char *end, size_t line)
{
	(void)condition;
	(void)begin;
	(void)end;
	(void)line;
}