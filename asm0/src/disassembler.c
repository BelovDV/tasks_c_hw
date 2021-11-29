#include "disassembler.h"
#include "dull.h"
#include "check.h"
#include "instruction.h"

#include <string.h>
#include <stdint.h>

// disassembler structure is very similar to assembler
// is it possible to use shared code?

static FILE *log_file = NULL;
static FILE *destination = NULL;

static int disassembler_core(Array_static_frame dest);

int disassemble(Array_static_frame bin, FILE *dest, FILE *log)
{
	fprintf(log, "Disassembler started working\n\n");

	check(bin.size != 0 && bin.array != NULL); // debug_check?
	check(dest != NULL);
	check(log != NULL);

	log_file = log;
	destination = dest;

	int result = 1;

	struct Dull *dull = bin.array;
	if (dull->header.magic[0] != 'd' || dull->header.magic[1] != 'u' ||
		dull->header.magic[2] != 'l' || dull->header.magic[3] != 'l')
		fprintf(log, "\nWrong magic bytes\n");
	else if (dull->version != ASM0_VERSION)
		fprintf(log, "\nWrong version\n");
	else
		result = disassembler_core(bin);
	if (result)
		fprintf(log_file, "\nDisassembling completed with errors\n");

	fprintf(log, "\nDisassembler finished working\n");

	return result;
}

static void dis_print_line(Instruction *instr)
{
	int line_size = 0;
	fprintf(destination, "\t%-*s%n", e_lang_max_word_len,
			language_instructions[instr->id].mnemonic, &line_size);

	for (int arg = 0; arg < e_const_max_arg_count; ++arg)
	{
		line_size = 0;
		if (arg < instr->argc)
		{
			Argument cur = instr->argv[arg];
			if (cur.content & e_arg_is_memory)
				fprintf(destination, "["), ++line_size;

			if (cur.content & e_arg_is_rx)
			{
				fprintf(destination, "r%d", cur.rx);
				line_size += 2;
				if (cur.content & (e_arg_is_ry |
								   e_arg_is_const))
				{
					fprintf(destination, "+");
					line_size += 1;
				}
			}
			if (cur.content & e_arg_is_ry)
			{
				fprintf(destination, "%d*r%d", cur.k, cur.ry);
				line_size += 4;
				if (cur.content & e_arg_is_const)
				{
					fprintf(destination, "+");
					line_size += 1;
				}
			}
			if (cur.content & e_arg_is_const)
			{
				int vsp;
				fprintf(destination, "%lu%n", cur.c, &vsp);
				line_size += vsp;
			}

			if (cur.content & e_arg_is_memory)
				fprintf(destination, "] "), line_size += 2;
		}

		while (line_size++ < e_lang_max_word_len)
			fprintf(destination, " ");
	}
}

static int disassembler_core(Array_static_frame bin)
{
	fprintf(destination, "# asm0 version %d\n\n", ASM0_VERSION);

	struct Dull *dull = bin.array;

	bin.array += sizeof(struct Dull);
	bin.size -= sizeof(struct Dull);
	Instruction instruction;
	uint8_t *begin = bin.array;
	uint8_t *end = begin + bin.size;

	for (uint8_t *iter = begin; iter < end;)
	{
		LOG_L(iter, "%p")
		LOG_I
		LOG_L(iter - begin, "0x%lx")
		size_t size = instr_decode(iter, &instruction);

		instr_dump(log_file, &instruction, "    ");
		if (size == 0)
			return 1;

		if ((size_t)(iter - begin) == dull->start_offset)
			fprintf(destination, "_start\n");
		dis_print_line(&instruction);

		fprintf(destination, " # 0x%08lx   ", iter - begin);
		print_raw_data(destination, iter, size, 0);
		fprintf(destination, "\n");

		fprintf(log_file, " # 0x%08lx   ", iter - begin);
		print_raw_data(log_file, iter, size, 0);
		fprintf(log_file, "\n");

		iter += size;
		LOG_D
	}

	return 0;
}