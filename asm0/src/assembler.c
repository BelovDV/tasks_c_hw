#include "assembler.h"
#include "check.h"
#include "utility.h"
#include "tokenization.h"
#include "language.h"

#include <string.h>

static FILE *log_file;

static int assembler_core(Text_string_static text);
static void assembler_saver(struct Dull **dest, size_t *size);
static void assembler_code_instr(struct Tokenization_condition *condition);

static size_t text_size = 0;
static size_t text_capacity = 0;
static uint8_t *text = NULL;
static size_t start_offset = -1;

int assemble(Text_string_static text, struct Dull **dest,
			 FILE *log, size_t *size)
{
	fprintf(log, "Assembler started working\n");

	check(text.length != 0);
	check(text.string != NULL);
	check(dest != NULL);
	check(*dest == NULL);
	check(log != NULL);

	log_file = log;

	int result = assembler_core(text);

	if (result)
		fprintf(log_file, "Assembling of program completed with errors\n");
	else
		assembler_saver(dest, size);

	fprintf(log, "Assembler finished working\n");

	return result;
}

static int assembler_core(Text_string_static text)
{
	struct Tokenization_condition *decoder =
		wrapped_calloc(1, sizeof(*decoder));
	tokenization_initialise(decoder);
	Text_string_index *indexation = NULL;
	size_t index_size = text_get_division_with_empty(text, '\n', &indexation);
	int error = 0;

	for (size_t line = 0; line < index_size; ++line)
	{
		tokenization_decode(
			decoder,
			text.string + indexation[line].index,
			text.string + indexation[line].index + indexation[line].length,
			line);
		if (decoder->error_id != e_tokenization_error_nothing)
		{
			error = 1;
			fprintf(log_file, "ERROR: %s\n", token_errors[decoder->error_id]);
			decoder->error_id = e_tokenization_error_nothing;
		}
		if ((decoder->flags & e_token_flag_mask_start) && start_offset == -1)
			start_offset = text_size;

		if (decoder->flags & e_token_flag_mask_intstr_changed)
			assembler_code_instr(decoder);
	}

	free(decoder);
	return error;
}

static void assembler_text_add(size_t bytes, void *data)
{
	if (text_size + bytes > text_capacity)
	{
		uint8_t *vsp = realloc(text, (text_size + bytes) * 2);
		check(vsp != NULL && "realloc");
		text = vsp;
	}
	memcpy(text + text_size, data, bytes);
	text_size += bytes;
}

static void assembler_instr_long_args(struct Argument *arg)
{
	uint8_t data = arg->k << 4;
	if (arg->is_memory)
		data |= e_argument_mask_is_memory;
	if (arg->rx)
		data |= e_argument_mask_is_rx;
	if (arg->ry)
		data |= e_argument_mask_is_ry;
	if (arg->constant)
		data |= e_argument_mask_is_const;
	uint8_t reg = (arg->rx << 4) + arg->ry;
	assembler_text_add(1, &data);
	assembler_text_add(1, &reg);
	if (text[0] & e_argument_mask_is_const)
		assembler_text_add(8, &arg->constant);
}

static void assembler_code_instr(struct Tokenization_condition *condition)
{
	assembler_text_add(1, &condition->last_instr.id);
	switch (condition->last_instr.id)
	{
	case e_instr_id_mov:
		assembler_instr_long_args(&condition->last_instr.argv[0]);
		assembler_instr_long_args(&condition->last_instr.argv[1]);
		break;
	case e_instr_id_libcall:
	case e_instr_id_syscall:
		assembler_text_add(1, &condition->last_instr.argv[0].constant);
		break;
	default:
		fprintf(log_file, "instruction %d doesn't supported\n",
				condition->last_instr.id);
	}
}

static void assembler_saver(struct Dull **dest, size_t *size)
{
	*dest = wrapped_calloc(1, sizeof(**dest) + text_size);
	(*dest)->header.magic[0] = 'd';
	(*dest)->header.magic[1] = 'u';
	(*dest)->header.magic[2] = 'l';
	(*dest)->header.magic[3] = 'l';
	(*dest)->start_offset = start_offset;
	uint8_t *data = (uint8_t *)*dest;
	memcpy(data + sizeof(*dest), text, text_size);
	*size = text_size + sizeof(**dest);
	printf("start: %lu\n", start_offset);
	printf("size: %lu\n", *size);
}