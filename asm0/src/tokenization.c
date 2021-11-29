#include "tokenization.h"
#include "language.h"
#include "check.h"

#include <stdlib.h>
#include <string.h>

const char token_errors[e_tok_error_count][64] =
	{
		"nothing wrong",
		"unsupported",
		"deprecated",
		"too_long_line",
		"multiply_start",
		"wrong_line_start",
		"wrong_instr",
		"wrong_arg",
		"wrong_label"};

void tokenization_initialise(Tokenization_condition *condition)
{
	debug_check(condition != NULL);
	condition->dictionary.array = NULL;
	condition->dictionary.size = 0;
	condition->dictionary.capacity = 0;
	condition->flags = 0;
	condition->last_instr.id = e_lang_instr_error;
}

static void token_decode_data(
	Tokenization_condition *condition,
	const char *begin, const char *end);

static void token_decode_label(
	Tokenization_condition *condition,
	const char *begin, const char *end);

void tokenization_decode(Tokenization_condition *condition,
						 const char *begin, const char *end,
						 int second)
{
	condition->flags &= ~e_tok_mask_changed_data;
	condition->flags &= ~e_tok_mask_changed_instr;
	condition->flags &= ~e_tok_mask_changed_label;
	condition->flags &= ~e_tok_mask_ready_jmp;
	condition->error_id = e_tok_error_nothing;

	debug_check(condition != NULL);
	debug_check(begin != NULL);
	debug_check(end != NULL);
	debug_check(begin <= end);

	for (const char *iter = end - 1; iter >= begin; --iter)
		if (*iter == '#')
			end = iter;
	while (end > begin && (end[-1] == ' ' || end[-1] == '\t'))
		--end;
	if (end == begin)
		return;

	if (end - begin >= e_lang_max_line_length)
	{
		condition->error_id = e_tok_error_too_long_line;
		return;
	}

	if (*begin == ':')
		token_decode_label(condition, begin, end);
	else if (*begin == '_')
	{
		LOG_L(*begin, "%c")
		if (condition->flags & e_tok_mask_start_found)
			condition->error_id = e_tok_error_multiply_start;
		condition->flags |= e_tok_mask_start_found;
		LOG_L(condition->flags, "0x%x")
	}
	else if (*begin == '\t')
		token_decode_data(condition, begin, end);
	else
		condition->error_id = e_tok_error_wrong_line_start;

	if (condition->error_id != e_tok_error_nothing)
	{
		LOG_LL(begin, (int)(end - begin), "%.*s")
		LOG_L(token_errors[condition->error_id], "%s")
	}
	if (!second && condition->error_id & e_tok_error_wrong_label)
	{
		condition->flags |= e_tok_mask_changed_instr;
	}
}

// ===== // ===== // ===== // ===== // ===== // ===== // ===== // ===== //

/**
 * @brief parse [rx+k*ry+c] string to Argument
 * @return was error (error - 1)
 */
static int token_parse_arg(const char *name, Argument *result)
{
	// yes, i know, this function should be subdivided to several smaller... but it isn't true, because it's parts are too specific
	// in a good way, there should be true regex, but it's too complex problem for me to work on it now
	result->c = 0;
	result->content = 0;
	result->k = 0;
	result->rx = 7;
	result->ry = 0;

	LOG_L(name, "%s")
	LOG_I
	size_t length = strlen(name);
	result->content = 0;
	if (*name == '[')
	{
		if (name[length - 1] != ']')
			return 1;
		++name;
		length -= 2;
		result->content |= e_arg_is_memory;
		LOG_L("", "is memory%s")
	}
	if (*name == 'r')
	{
		int rx, change;
		int res = sscanf(name, "r%d%n", &rx, &change);
		result->rx = (char)rx;
		LOG_L(result->rx, "%d")
		if (res < 1)
			return 1;
		name += change;
		length -= change;
		result->content |= e_arg_is_rx;
		if (length == 0)
		{
			LOG_D
			return 0;
		}
		if (*name != '+')
			return 1;
		name += 1;
		length -= 1;
	}
	if (strcnt(name, '*'))
	{
		int k, ry, change;
		int res = sscanf(name, "%d*r%d%n", &k, &ry, &change);
		if (res < 2)
			return 1;
		result->k = k;
		result->ry = ry;
		name += change;
		length -= change;
		result->content |= e_arg_is_ry;
		if (length == 0)
		{
			LOG_D
			return 0;
		}
		if (*name != '+')
			return 1;
		name += 1;
		length -= 1;
	}
	result->content |= e_arg_is_const;
	int change;
	int res = sscanf(name, "%lu%n", &result->c, &change);

	LOG_D
	if (res < 1 || length - change != 0)
		return 1;
	return 0;
}

/**
 * @brief find keyword id in language
 */
static int token_keyword_id(const char *name)
{
	for (int iter = 0; iter < e_lang_key_count; ++iter)
		if (strcmp(language_keywords[iter], name) == 0)
			return iter;
	return e_lang_key_error;
}

/**
 * @brief find instruction id in language
 */
static int token_instruction_id(const char *name)
{
	for (int iter = 0; iter < e_lang_instr_count; ++iter)
		if (strcmp(language_instructions[iter].mnemonic, name) == 0)
			return iter;
	return e_lang_instr_error;
}

static void token_decode_data(
	Tokenization_condition *condition,
	const char *begin, const char *end)
{
	char buffer[e_lang_max_line_length + 1];
	char word[5][e_lang_max_line_length + 1];
	word[0][0] = word[1][0] = word[2][0] = word[3][0] = word[4][0] = '\0';

	memcpy(buffer, begin, end - begin);
	buffer[end - begin] = '\0';

	int word_count = sscanf(
		buffer, "%s%s%s%s%s",
		word[0], word[1], word[2], word[3], word[4]);
	LOG_L(word[0], "%s")
	LOG_I
	Instruction *instruction = &condition->last_instr;

	if (strcmp(word[0], "jmp") == 0 || strcmp(word[0], "call") == 0)
	{
		if (strcmp(word[0], "jmp") == 0)
			condition->last_instr.id = e_lang_instr_jmp;
		else
			condition->last_instr.id = e_lang_instr_call;
		Label *labels = condition->dictionary.array;
		if (word_count == 3)
		{
			if (strcmp(word[1], "1") == 0)
				instruction->argv[0].k = 1;
			else if (strcmp(word[1], "2") == 0)
				instruction->argv[0].k = 2;
			else if (strcmp(word[1], "4") == 0)
				instruction->argv[0].k = 4;
			else if (strcmp(word[1], "8") == 0)
				instruction->argv[0].k = 8;
			else
			{
				condition->error_id = e_tok_error_wrong_arg;
				return;
			}
			for (size_t i = 0; i < condition->dictionary.size; ++i)
			{
				instruction->argc = 1;
				instruction->argv[0].content = e_arg_is_const;

				if (strcmp(word[2], labels[i].name) == 0)
				{
					LOG_L(word[1], "%s")
					LOG_L(instruction->argv[0].k, "jmp %hhd")
					instruction->argv[0].c = labels[i].position;
					LOG_L(word[2], "%s")
					LOG_L(labels[i].position, "0x%lx")
					condition->flags |= e_tok_mask_changed_instr;
					LOG_D
					return;
				}
			}
		}
		else if (word_count == 2)
		{
			printf("word count = 2\n");
			size_t input;
			if (!sscanf(word[1], "%lu", &input))
			{
				condition->error_id = e_tok_error_wrong_arg;
				printf("ERROR\n");
				return;
			}
			printf("word 1: %lu\n", input);
			printf("word 1: %ld\n", (Word)input);
			instruction->argc = 1;
			instruction->argv[0].content = input;
			instruction->argv[0].k = -128;
			condition->flags |= e_tok_mask_ready_jmp;
			condition->flags |= e_tok_mask_changed_instr;
			LOG_L(word[1], "%s")
			return;
		}
		LOG_L(word[2], "label %s wasn't found") // or k != 1, 2, 4, 8
		LOG_D
		condition->error_id = e_tok_error_wrong_label;
		return;
	}

	instruction->id = token_instruction_id(word[0]);
	if (instruction->id == e_lang_key_error)
	{
		condition->error_id = e_tok_error_wrong_instr;
		return;
	}
	instruction->argc = word_count - 1;
	for (int i = 1; i < word_count; ++i)
	{
		LOG_L(word[i], "%s")
		int keyword = token_keyword_id(word[i]);
		if (keyword == e_lang_key_error)
		{
			if (token_parse_arg(word[i], &instruction->argv[i - 1]))
			{
				condition->error_id = e_tok_error_wrong_arg;
				return;
			}
			LOG_L(instruction->argv[i - 1].content, "%d")
		}
		else
		{
			LOG_L(keyword, "%d")
			instruction->argv[i - 1].content = e_arg_is_const;
			instruction->argv[i - 1].c = keyword;
		}
	}
	condition->flags |= e_tok_mask_changed_instr;

	LOG_L(instruction->argc, "%d")

	LOG_D
}

static void token_decode_label(
	Tokenization_condition *condition,
	const char *begin, const char *end)
{
	char buffer[e_lang_max_line_length + 1];
	char word[5][e_lang_max_line_length + 1];
	word[0][0] = word[1][0] = word[2][0] = word[3][0] = word[4][0] = '\0';

	memcpy(buffer, begin, end - begin);
	buffer[end - begin] = '\0';

	int word_count = sscanf(
		buffer, "%s%s%s%s%s",
		word[0], word[1], word[2], word[3], word[4]);
	LOG_L(word[0], "%s")
	LOG_I

	if (word_count != 1 || strlen(word[0]) == 1)
	{
		condition->error_id = e_tok_error_wrong_label;
		LOG_L("", "wrong label%s")
		return;
	}

	Label *labels = condition->dictionary.array;
	for (size_t i = 0; i < condition->dictionary.size; ++i)
	{
		if (strcmp(word[0], labels[i].name) == 0)
		{
			labels[i].name[0] = '\0';
		}
	}

	if (condition->dictionary.size * sizeof(Label) == condition->dictionary.capacity)
	{
		LOG_L(condition->dictionary.capacity, "%lu")
		utility_reserve(
			&condition->dictionary,
			condition->dictionary.capacity * 2 + 2 * sizeof(Label));
	}

	Label *label = ((Label *)condition->dictionary.array) +
				   condition->dictionary.size;
	memcpy(label->name, word[0] + 1, strlen(word[0]) - 1);
	label->name[strlen(word[0]) - 1] = '\0';
	LOG_L(label->name, "%.20s")
	condition->dictionary.size++;
	condition->flags |= e_tok_mask_changed_label;
}