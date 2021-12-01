#include "assembler.h"
#include "language.h"
#include "check.h"
#include "dull.h"
#include "coder.h"
#include "log.h"

#include <string.h>

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
		LOG("%lu", W_COUNT)
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
		LOG("%d", (int)start)
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
	LOG("%lu", W_BEGIN)
	LOG("%lu", W_END)
	LOG("%lu", W_COUNT)
	for (size_t i = 0; i < W_COUNT; ++i)
	{
		LOG("%lu", i)
		if (WORD_SIZE(i) == 1 && WORD(i)[0] == '#')
		{
			W_COUNT = i - W_BEGIN;
			break;
		}
	}
	asm1->line += 1;
	LOG_OUT
	return 0;
}

static int decode_arg(Assembler *asm1, size_t word_number)
{
	LOG_IN
	LOG("%lu", word_number)
	int is_error = 0;

	// regex
	(void)asm1;

	LOG_OUT
	return is_error;
}

static int decode_instr(Assembler *asm1)
{
	LOG_IN
	CHECK_RETURN(W_COUNT <= e_max_arg_count + 1, "too many words")
	int is_error = 1;
	Line *instr = &asm1->lang_line;

	for (size_t id = 0; id < e_lang_instr_count; ++id)
		if (strncmp(language_mnemonics[id], WORD(0), WORD_SIZE(0)) == 0)
		{
			is_error = 0;
			instr->type = e_line_instr;
			break;
		}
	CHECK_RETURN(!is_error, "wrong instruction")

	for (size_t i = 1; i < W_COUNT; ++i)
		CHECK_RETURN(!decode_arg(asm1, i), "wrong arg")

	size_t sz = coder_code((char *)asm1->bin.array + POSITION, instr);
	CHECK_RETURN(sz > 0, "instruction coding error")
	log_line(asm1, sz);
	POSITION += sz;
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

	for (size_t i = 0; i < labels->size; ++i) // TODO - it's ugly
		CHECK_RETURN(!(strlen(labels->array[i].name) == WORD_SIZE(0) &&
					   strncmp(labels->array[i].name, WORD(0), WORD_SIZE(0) == 0)),
					 "equal labels")

	memset(utility_array_end(labels).name, 0, e_max_word_length);
	memcpy(utility_array_end(labels).name, WORD(0), WORD_SIZE(0));
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
	int position = 0, delta;
	fprintf(asm1->log, "%4lu: 0x%4.4lx: %n", asm1->line, POSITION, &delta);
	position += delta;
	for (size_t i = 0; i < W_COUNT; ++i, position += delta)
		fprintf(asm1->log, "'%.*s' %n", (int)WORD_SIZE(i), WORD(i), &delta);
	while (position++ < e_max_line_length)
		fprintf(asm1->log, " ");
	fprintf(asm1->log, " # ");
	print_raw_data(asm1->log, (char *)asm1->bin.array + POSITION, b_size);
	fprintf(asm1->log, "\n");
}

static void log_error(Assembler *asm1, const char *message)
{
	fprintf(asm1->log, "ERROR: line %lu: %s\n", asm1->line, message);
}

static void store(Assembler *asm1)
{
	LOG_IN(void)
	asm1;
	LOG_OUT
}

static int set_labels(Assembler *asm1)
{
	LOG_IN(void)
	asm1;
	LOG_OUT
	return 0;
}