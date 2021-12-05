#include "assembler.h"
#include "language.h"
#include "check.h"
#include "dull.h"
#include "coder.h"
#include "log.h"
#include "regex.h"

#include <string.h>

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

int regex(Regex *regex);

#endif

// ===== // DECLARATIONS // ===== //

typedef struct
{
	char name[e_max_word_length]; // +1'\0'-1':'
	size_t address;
} Label;

typedef struct
{
	Label *array;
	size_t size;
	size_t capacity;
} Labels;

typedef struct
{
	FILE *log;				 // not our responsibility
	const Text_string *text; // not our responsibility

	Text lines;		   // assemble
	Text words;		   // assemble
	size_t line;	   // decode_next_line
	size_t word_begin; // decode_next_line
	size_t word_end;   // decode_next_line
	size_t W_COUNT;	   // decode_next_line

	size_t start_offset; // decode_start

	Labels labels; // decode_label ; assemble
	Labels lab_requests;

	Array_frame bin;		  // assemble
	Array_static_frame *dest; // not our responsibility

	int section; // decode_section
	Line lang_line;
} Assembler; // assemble

static int decode(Assembler *asm1);
static void store(Assembler *asm1);

static int choose_line(Assembler *asm1);

static void log_line(Assembler *asm1, size_t b_size);
static void log_error(Assembler *asm1, const char *message);

static int decode_instr(Assembler *asm1);
static int decode_data(Assembler *asm1);
static int decode_start(Assembler *asm1);
static int decode_section(Assembler *asm1);
static int decode_label(Assembler *asm1);
static int decode_arg(Assembler *asm1, size_t word_number);
static int decode_jmp(Assembler *asm1, int instr_id);

static int set_labels(Assembler *asm1);

#define CHECK_RETURN(condition, message) \
	{                                    \
		if (!(condition))                \
		{                                \
			log_error(asm1, message);    \
			LOG_OUT                      \
			return 1;                    \
		}                                \
	}
#define CHECK_MESSAGE(condition, message) \
	{                                     \
		if (!(condition))                 \
		{                                 \
			is_error = 1;                 \
			log_error(asm1, message);     \
		}                                 \
	}

#define WORD(number) (asm1->words.index[W_BEGIN + number].value)
#define WORD_SIZE(number) (asm1->words.index[W_BEGIN + number].size)
#define POSITION (asm1->bin.size)
#define W_COUNT (asm1->W_COUNT)
#define W_BEGIN (asm1->word_begin)
#define W_END (asm1->word_end)

// ===== // GLOBAL // ===== //

int assemble(const Text_string *text, Array_static_frame *bin, FILE *log)
{
	LOG_IN
	debug_check(text != NULL && bin != NULL && log != NULL);
	fprintf(log, "Assembler started working\n\n");

	Assembler *asm1 = calloc(1, sizeof(Assembler));
	asm1->log = log;
	asm1->text = text;
	asm1->dest = bin;
	asm1->lines = text_decompose(asm1->text, "\n", 0);
	asm1->words = text_decompose(asm1->text, "\n\t ", 1);

	int is_error = decode(asm1);
	if (!is_error)
	{
		store(asm1);
		fprintf(log, "\nStart offset is 0x%lx\n", asm1->start_offset);
	}
	else
		fprintf(log, "\nErrors were encountered\n");
	fprintf(log, "\nAssembler finished working\n");
	free(asm1->labels.array);
	free(asm1->lab_requests.array);
	free(asm1->lines.index);
	free(asm1->words.index);
	free(asm1->bin.array);
	free(asm1);
	LOG("%d", is_error)
	LOG_OUT
	return is_error;
}

// ===== // STATIC // ===== //

static int decode(Assembler *asm1)
{
	LOG_IN
	asm1->start_offset = (size_t)-1;
	asm1->section = e_section_none;
	int is_error = 0;
	LOG("%lu", asm1->words.size)
	LOG("%lu", asm1->lines.size)

	while (!choose_line(asm1))
	{
		LOG("%lu", asm1->line)
		// LOG("%lu", W_COUNT)
		utility_array_provide_space(&asm1->labels, sizeof(Label));
		while (asm1->bin.capacity - asm1->bin.size < e_max_line_length)
			utility_array_add_space(&asm1->bin, 1);
		if (W_COUNT == 0)
			continue;
		for (size_t i = 0; i < W_COUNT; ++i)
			CHECK_MESSAGE(strnlen_del(WORD(i), " \t\n", WORD_SIZE(i)) <=
							  e_max_word_length,
						  "too long word")
		char start = *asm1->lines.index[asm1->line - 1].value;
		// LOG("%d", (int)start)
		if (start == '.')
			is_error |= decode_section(asm1);
		else if (start == ':')
			is_error |= decode_label(asm1);
		else if (start == '_')
			is_error |= decode_start(asm1);
		else if (start == '\t')
		{
			if (asm1->section == e_section_text)
				is_error |= decode_instr(asm1);
			else if (asm1->section == e_section_data)
				is_error |= decode_data(asm1);
			else
				CHECK_MESSAGE(0, "cannot understand section")
		}
		else
			CHECK_MESSAGE(0, "cannot parse line")
	}
	is_error |= set_labels(asm1);
	CHECK_MESSAGE(asm1->start_offset != (size_t)-1, "cannot find start")
	LOG_OUT
	return is_error;
}

static int choose_line(Assembler *asm1) // TODO - there is smth bad here
{
	LOG_IN
	if (asm1->line >= asm1->lines.size)
	{
		LOG("%s", "last line")
		LOG_OUT
		return 1;
	}
	W_BEGIN = W_END;

	Text_string *line = asm1->lines.index + asm1->line;
	const char *line_end = line->value + line->size;

	while (W_END != asm1->words.size &&
		   asm1->words.index[W_END].value +
				   asm1->words.index[W_END].size <=
			   line_end)
		++W_END;

	W_COUNT = W_END - W_BEGIN;
	// LOG("%lu", W_BEGIN)
	// LOG("%lu", W_END)
	//  LOG("%lu", W_COUNT)
	for (size_t i = 0; i < W_COUNT; ++i)
		if (WORD_SIZE(i) == 1 && WORD(i)[0] == '#')
		{
			W_COUNT = i;
			break;
		}
	asm1->line += 1;
	LOG_OUT
	return 0;
}

static int decode_arg_r(char *r, char *result[])
{
	/*
	LOG_IN
	for (int i = 0; i < 6; ++i)
	{
		LOG("%p", result[i])
	}
	LOG_OUT
	*/

	if (result[0]) // is r
	{
		if (result[1])
			*r = e_exe_reg_r10 + result[1][1] - '0';
		else if (result[2])
			*r = e_exe_reg_r0 + result[2][0] - '0';
		else if (result[3])
			*r = e_exe_reg_rbp;
		else if (result[4])
			*r = e_exe_reg_rsp;
		else if (result[5])
			*r = e_exe_reg_rf;
		else
			return 1;
	}
	else
		*r = e_exe_reg_count;
	LOG("%d", (int)*r)
	return 0;
}

static int decode_arg(Assembler *asm1, size_t word_number)
{
	LOG_IN
	LOG("%lu", word_number)
	Argument *arg = &asm1->lang_line.instr.argv[word_number - 1];
	// LOG("%p", arg)
	for (int i = 0; i < e_lang_key_count; ++i)
		if (strlen(language_keywords[i]) == WORD_SIZE(word_number) &&
			strncmp(language_keywords[i], WORD(word_number),
					WORD_SIZE(word_number)) == 0)
		{
			arg->c = (size_t)i;
			arg->k = 0;
			arg->rx = e_exe_reg_count;
			arg->ry = e_exe_reg_count;
			arg->mem = 0;
			LOG_OUT
			return 0;
		}

	char buffer_rule[] =
		"?$["
		"?(r(!(1[0-1])|![0-9]|!(bp)|!(sp)|!f))"
		"?+"
		"?((!1|!2|!4|!8)$*!r(!(1[0-1])|![0-9]|!(bp)|!(sp)|!f))"
		"?+"
		"?([1-9]*[0-9])"
		"?$]";
	char buffer_test[256] = "";
	memcpy(buffer_test, WORD(word_number), WORD_SIZE(word_number));
	LOG("%s", buffer_test)
	char *result[256];
	Regex reg = {buffer_rule, 0, result, buffer_test, 0};
	CHECK_RETURN(!regex(&reg), "cannot parse arg")

	/*
		for (int i = 0; i < 22; ++i)
		{
			LOG("%p", result[i])
		}
	*/

	CHECK_RETURN((result[0] == NULL) == (result[21] == NULL), "mismatch []")
	if (result[0])
		arg->mem = 1;
	else
		arg->mem = 0;
	CHECK_RETURN(!decode_arg_r(&arg->rx, result + 1), "wrong rx")
	if (result[8]) // is k
	{
		// CHECK_RETURN(arg->rx == e_exe_reg_count || result[7],
		//			 "missing '+' between rx k")
		arg->k = result[8][0] - '0';
		CHECK_RETURN(!decode_arg_r(&arg->ry, result + 13), "wrong ry")
	}
	else
		arg->k = 0;
	if (result[20])
	{
		// CHECK_RETURN(arg->k == 0 )
		arg->is_c = 1;
		sscanf(result[20], "%lu", &arg->c);
	}
	else
		arg->is_c = 0;
	LOG("%d", (int)arg->is_c)

	/*
		LOG("%d", (int)arg->mem)
		LOG("%d", (int)arg->rx)
		LOG("%d", (int)arg->k)
		LOG("%d", (int)arg->ry)
		LOG("%lu", arg->c)
		*/

	int is_error = 0;
	LOG_OUT
	return is_error;
}

static int decode_jmp(Assembler *asm1, int instr_id)
{
	LOG_IN
	CHECK_RETURN(W_COUNT == 3, "wrong jmp/call wc")
	Line *line = &asm1->lang_line;
	line->type = e_line_instr;
	Instruction *instr = &asm1->lang_line.instr;
	memset(instr, 0, sizeof(Instruction));
	instr->argv[0].rx = instr->argv[1].rx = e_exe_reg_count;
	instr->id = instr_id;
	instr->argc = 2;
	instr->argv[1].c = (size_t)-1;
	instr->argv[0].is_c = instr->argv[1].is_c = 1;
	int is_error = 0;
	if (!sscanf(WORD(1), "%lu", &instr->argv[0].c))
		is_error = 1;
	utility_array_provide_space(&asm1->lab_requests, sizeof(Label));
	Labels *requests = &asm1->lab_requests;
	memset(utility_array_end(requests).name, 0, e_max_word_length);
	memcpy(utility_array_end(requests).name, WORD(2), WORD_SIZE(2));
	utility_array_end(requests).address = POSITION;
	LOG("%s", utility_array_end(requests).name)
	// LOG("%p", utility_array_end(requests).position)
	requests->size += 1;
	size_t sz = coder_code((char *)asm1->bin.array + POSITION, line);
	CHECK_RETURN(sz > 0, "instruction coding error")
	log_line(asm1, sz);
	POSITION += sz;
	LOG_OUT
	return is_error;
}

static int decode_instr(Assembler *asm1)
{
	LOG_IN
	CHECK_RETURN(W_COUNT <= e_max_arg_count + 1, "too many words")
	int is_error = 1;
	Line *line = &asm1->lang_line;

	if (strlen(mnemonics[e_lang_instr_jmp]) == WORD_SIZE(0) &&
		strncmp(mnemonics[e_lang_instr_jmp], WORD(0), WORD_SIZE(0)) == 0)
	{
		LOG("jmp%s", "")
		is_error = decode_jmp(asm1, e_lang_instr_jmp);
		LOG_OUT
		return is_error;
	}
	if (strlen(mnemonics[e_lang_instr_call]) == WORD_SIZE(0) &&
		strncmp(mnemonics[e_lang_instr_call], WORD(0), WORD_SIZE(0)) == 0)
	{
		LOG("call%s", "")
		is_error = decode_jmp(asm1, e_lang_instr_call);
		LOG_OUT
		return is_error;
	}
	for (size_t id = 0; id < e_lang_instr_count; ++id)
		if (strncmp(mnemonics[id], WORD(0), WORD_SIZE(0)) == 0)
		{
			is_error = 0;
			line->type = e_line_instr;
			line->instr.id = id;
			break;
		}
	CHECK_RETURN(!is_error, "wrong instruction")

	Instruction *instr = &line->instr;
	instr->argc = W_COUNT - 1;
	for (size_t i = 1; i < W_COUNT; ++i)
		CHECK_RETURN(!decode_arg(asm1, i), "wrong arg")

	// for (size_t i = 0; i < instr->argc; ++i)
	// {
	// 	LOG("argn %lu", i)
	// 	LOG("%d", (int)instr->argv[i].rx)
	// 	LOG("%d", (int)instr->argv[i].k)
	// 	LOG("%d", (int)instr->argv[i].ry)
	// 	LOG("%lu", instr->argv[i].c)
	// }

	size_t sz = coder_code((char *)asm1->bin.array + POSITION, line);
	CHECK_RETURN(sz > 0, "instruction coding error")
	log_line(asm1, sz);
	LOG("%lu", POSITION)
	POSITION += sz;
	LOG("%lu", POSITION)
	LOG_OUT
	return 0;
}

static int decode_data(Assembler *asm1)
{
	LOG_IN
	CHECK_RETURN(strncmp(WORD(0), "string", 6) == 0, "wrong data line")
	CHECK_RETURN(W_COUNT == 2, "wrong count of words")

	Line *data = &asm1->lang_line;
	data->type = e_line_data;
	data->data.size = WORD_SIZE(1);
	memset(data->data.data, 0, e_max_word_length + 1);
	memcpy(data->data.data, WORD(1), WORD_SIZE(1));

	size_t sz = coder_code((char *)asm1->bin.array + POSITION, data);
	CHECK_RETURN(sz > 0, "data coding error")
	log_line(asm1, sz);
	POSITION += sz;
	LOG_OUT
	return 0;
}

static int decode_start(Assembler *asm1)
{
	LOG_IN
	CHECK_RETURN(W_COUNT == 1, "too many words")
	CHECK_RETURN(strncmp(WORD(0), "_start", 6) == 0, "wrong start line")
	CHECK_RETURN(asm1->start_offset == (size_t)-1, "multiply starts")
	asm1->start_offset = asm1->bin.size;
	log_line(asm1, 0);
	LOG_OUT
	return 0;
}

static int decode_section(Assembler *asm1)
{
	LOG_IN
	CHECK_RETURN(W_COUNT == 1, "too many words")
	if (strncmp(WORD(0), ".TEXT", 5) == 0)
		asm1->section = e_section_text;
	else if (strncmp(WORD(0), ".DATA", 5) == 0)
		asm1->section = e_section_data;
	else
		CHECK_RETURN(0, "wrong section line")
	log_line(asm1, 0);
	LOG_OUT
	return 0;
}

static int decode_label(Assembler *asm1)
{
	LOG_IN
	CHECK_RETURN(W_COUNT == 1, "too many words")
	Labels *labels = &asm1->labels;

	for (size_t i = 0; i < labels->size; ++i)
		CHECK_RETURN(!(strlen(labels->array[i].name) == WORD_SIZE(0) &&
					   strncmp(labels->array[i].name, WORD(0), WORD_SIZE(0)) == 0),
					 "equal labels")

	memset(utility_array_end(labels).name, 0, e_max_word_length);
	memcpy(utility_array_end(labels).name, WORD(0) + 1, WORD_SIZE(0) - 1);
	utility_array_end(labels).address = POSITION;
	LOG("%s", utility_array_end(labels).name)
	labels->size += 1;
	log_line(asm1, 0);
	LOG("%lu", POSITION)
	LOG_OUT
	return 0;
}

static void log_line(Assembler *asm1, size_t b_size)
{
	LOG("%lu", b_size)
	int position = 0, delta;
	fprintf(asm1->log, "%4lu: 0x%4.4lx: %n", asm1->line, POSITION, &delta);
	position += delta;
	for (size_t i = 0; i < W_COUNT; ++i, position += delta)
		fprintf(asm1->log, "'%.*s' %n", (int)WORD_SIZE(i), WORD(i), &delta);
	while (position++ < e_max_line_length)
		fprintf(asm1->log, " ");
	fprintf(asm1->log, " # 0x: ");
	print_raw_data(asm1->log, (char *)asm1->bin.array + POSITION, b_size);
	fprintf(asm1->log, "\n");
}

static void log_error(Assembler *asm1, const char *message)
{
	fprintf(asm1->log, "ERROR: line %lu: %s\n", asm1->line, message);
}

static void store(Assembler *asm1)
{
	LOG_IN;
	Array_frame *bin = &asm1->bin;
	Array_static_frame *dest = asm1->dest;
	dest->size = bin->size + sizeof(struct Dull);
	dest->array = calloc(1, dest->size);
	struct Dull *header = dest->array;
	header->header.magic[0] = 'd';
	header->header.magic[1] = 'u';
	header->header.magic[2] = 'l';
	header->header.magic[3] = 'l';
	header->header.id = 1;
	header->start_offset = asm1->start_offset;
	char *data = (char *)(header + 1);
	memcpy(data, bin->array, bin->size);
	LOG_OUT
}

static int set_labels(Assembler *asm1)
{
	LOG_IN
	for (size_t i = 0; i < asm1->lab_requests.size; ++i)
	{
		Label *request = &asm1->lab_requests.array[i];
		void *position = (char *)asm1->bin.array + request->address;
		size_t label_position = (size_t)-1;
		LOG("%s", request->name)
		LOG("%lu", request->address)
		for (size_t i = 0; i < asm1->labels.size; ++i)
			if (strlen(asm1->labels.array[i].name) == strlen(request->name) &&
				strncmp(asm1->labels.array[i].name, request->name, strlen(request->name)) == 0)
			{
				label_position = asm1->labels.array[i].address;
				break;
			}
			else
			{
				// LOG("%s", asm1->labels.array[i].name)
			}
		LOG("%lu", label_position)
		CHECK_RETURN(label_position != (size_t)-1, "cannot find label")
		coder_modify_jump(position, label_position - request->address);
	}
	LOG_OUT
	return 0;
}