#include "assembler.h"
#include "check.h"
#include "tokenization.h"
#include "dull.h"

#include <string.h>
#include <stdint.h>

static FILE *log_file;

static int assembler_core(Text_string text);
static void assembler_saver(Array_static_frame *dest);

static Array_frame bin = {0, 0, 0};
static size_t start_offset = -1;

int assemble(Text_string text, Array_static_frame *dest, FILE *log)
{
	fprintf(log, "Assembler started working\n\n");

	check(text.size != 0); // debug_check?
	check(text.value != NULL);
	check(dest != NULL);
	check(dest->size == 0 && dest->array == NULL);
	check(log != NULL);

	log_file = log;

	int result = assembler_core(text);

	if (result)
		fprintf(log_file, "\nAssembling of program completed with errors\n");
	else
		assembler_saver(dest);

	fprintf(log, "\nAssembler finished working\n");

	free(bin.array); // sanitizer doesn't notice, should i free static var?

	return result;
}

static int assembler_core(Text_string text)
{
	Tokenization_condition *decoder = wrapped_calloc(decoder, 1);
	tokenization_initialise(decoder);
	Text index = text_decompose(text, "\n", 0);

	for (size_t line = 0; line < index.size; ++line)
	{
		LOG_L(line, "%lu");
		LOG_I;
		tokenization_decode(
			decoder,
			index.indexation[line].value,
			index.indexation[line].value + index.indexation[line].size,
			0);
		LOG_D;

		size_t previous_size = bin.size;
		if (decoder->flags & e_tok_mask_changed_instr)
		{
			if (decoder->flags & e_tok_mask_ready_jmp)
				decoder->last_instr.argv[0].rx = 1;
			else
				decoder->last_instr.argv[0].rx = 0;
			instr_code(&decoder->last_instr, &bin);
			fprintf(
				log_file,
				"0x%8lx: %-10.10s",
				previous_size,
				language_instructions[decoder->last_instr.id].mnemonic);
			print_raw_data(
				log_file,
				(uint8_t *)bin.array + previous_size,
				bin.size - previous_size, 0);
			fprintf(log_file, "\n");
			LOG_L((*((uint8_t *)bin.array + previous_size)), "%hhd")
		}
		if (decoder->flags & e_tok_mask_changed_label)
		{
			((Label *)decoder->dictionary.array)[decoder->dictionary.size - 1].position = bin.size;
			LOG_L(bin.size, "label position %lx")
			fprintf(log_file, "label 0x%lx\n", bin.size);
		}
	}
	decoder->flags &= !e_tok_mask_start_found;
	bin.size = 0;
	bin.capacity = 0;
	free(bin.array);
	bin.array = NULL;

	for (size_t i = 0; i < decoder->dictionary.size; ++i)
		fprintf(
			log_file,
			"old label %3lu: 0x%4lx: %s\n", i,
			((Label *)decoder->dictionary.array)[i].position,
			((Label *)decoder->dictionary.array)[i].name);

	int error = 0;
	for (size_t line = 0; line < index.size; ++line)
	{
		LOG_L(line, "%lu");
		LOG_I;
		tokenization_decode(
			decoder,
			index.indexation[line].value,
			index.indexation[line].value + index.indexation[line].size,
			1);
		LOG_D;
		error |= decoder->error_id != e_tok_error_nothing;
		if (decoder->error_id != e_tok_error_nothing)
			fprintf(
				log_file,
				"ERROR: line %3lu:\t %s\n"
				"\t'%.*s'\n",
				line, token_errors[decoder->error_id],
				(int)index.indexation[line].size, index.indexation[line].value);

		if ((decoder->flags & e_tok_mask_start_found) && start_offset == -1UL)
		{
			start_offset = bin.size;
			LOG_L(start_offset, "0x%lx")
			fprintf(log_file, "start offset %lu\n", start_offset);
		}
		size_t previous_size = bin.size;
		if (decoder->flags & e_tok_mask_changed_instr)
		{
			if (decoder->last_instr.id == e_lang_instr_jmp || decoder->last_instr.id == e_lang_instr_call)
			{
				if (decoder->flags & e_tok_mask_ready_jmp)
					decoder->last_instr.argv[0].rx = 1;
				else
					decoder->last_instr.argv[0].rx = 0;
			}
			if (instr_code(&decoder->last_instr, &bin))
			{
				error = 1;
				fprintf(log_file,
						"ERROR: line %3lu:\t %s\n",
						line, "cannot decode instruction");
			}
			else
			{
				fprintf(
					log_file,
					"0x%8lx: %-10.10s",
					previous_size,
					language_instructions[decoder->last_instr.id].mnemonic);
				print_raw_data(
					log_file,
					(uint8_t *)bin.array + previous_size,
					bin.size - previous_size, 0);
				fprintf(log_file, "\n");
			}
			LOG_L((*((uint8_t *)bin.array + previous_size)), "%hhd")
		}
		if (decoder->flags & e_tok_mask_changed_label)
		{
			((Label *)decoder->dictionary.array)[decoder->dictionary.size - 1].position = bin.size;
			LOG_L(bin.size, "label position %lx")
			fprintf(log_file, "label 0x%lx\n", bin.size);
		}
	}

	if (start_offset == (size_t)-1)
	{
		fprintf(log_file, "Cannot find start\n");
		error = 1;
	}

	free(decoder->dictionary.array);
	free(index.indexation);
	free(decoder);
	return error;
}

static void assembler_saver(Array_static_frame *dest)
{
	dest->size = bin.size + sizeof(struct Dull);
	wrapped_calloc(dest->array, dest->size);
	struct Dull *header = dest->array;
	header->header.magic[0] = 'd';
	header->header.magic[1] = 'u';
	header->header.magic[2] = 'l';
	header->header.magic[3] = 'l';
	header->version = ASM0_VERSION;
	header->start_offset = start_offset;
	uint8_t *data = (uint8_t *)(header + 1);
	memcpy(data, bin.array, bin.size);
	LOG_L(bin.size, "%lu");
}