#include "executor.h"
#include "check.h"
#include "instruction.h"
#include "dull.h"

#include <stdint.h>
#include <string.h>

static FILE *file_log = NULL;
static Executor exe = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

#define RIP (exe.r[e_exe_reg_rip])
#define RBP (exe.r[e_exe_reg_rbp])
#define RSP (exe.r[e_exe_reg_rsp])
#define RF (exe.r[e_exe_reg_rf])

static int exe_execute();

int execute(Array_static_frame code, FILE *log)
{
	debug_check(log != NULL);
	debug_check(code.array != NULL);

	if (code.size > CODE_SIZE)
	{
		fprintf(log, "ERROR: too long code\n");
		return 1;
	}

	file_log = log;

	wrapped_calloc(exe.memory, MEMORY_SIZE);
	memcpy((uint8_t *)exe.memory + STACK_SIZE, (uint8_t *)code.array + sizeof(struct Dull), code.size - sizeof(struct Dull));
	RIP = STACK_SIZE + ((struct Dull *)code.array)->start_offset;

	int result = exe_execute();
	executor_dump(file_log, &exe, 0, result ? 2 * STACK_SIZE : 0);

	free(exe.memory); // exe static - should i free?

	return result;
}

static int exe_execute()
{
	Instruction instr;
	uint8_t *code = (uint8_t *)exe.memory;
	while (1)
	{
		LOG_L(RIP, "0x%lx")
		LOG_I
		executor_dump(file_log, &exe, 0, 0);
		size_t instr_sz = instr_decode(code + RIP, &instr);
		LOG_L(language_instructions[instr.id].mnemonic, "%s")
		instr_dump(file_log, &instr, "\t\t\t\t");
		char string_error[] = "ERROR: %s\n";
		if (instr.id >= e_lang_instr_count || instr_sz == 0)
		{
			fprintf(file_log, string_error, "cannot decode instruction");
			return 1;
		}
		RIP += instr_sz;
		if (language_instructions[instr.id].exe(&exe, &instr))
		{
			if (exe.error & e_exe_error_memory_access_violation)
				fprintf(file_log, string_error, "memory access violation");
			if (exe.error & e_exe_error_wrong_word_type)
				fprintf(file_log, string_error, "wrong_word_type");
			if (exe.error & e_exe_error_unsupported)
				fprintf(file_log, string_error, "unsupported");
			if (exe.error & e_exe_error_wrong_keyword)
				fprintf(file_log, string_error, "wrong_keyword");
			if (exe.error & e_exe_error_wrong_argc)
				fprintf(file_log, string_error, "wrong count of args");
			if (exe.error & e_exe_error_wrong_arg_type)
				fprintf(file_log, string_error, "wrong argument type");
			if (exe.error & e_exe_error_wrong_const)
				fprintf(file_log, string_error, "wrong const");
			if (exe.error & e_exe_error_zero_division)
				fprintf(file_log, string_error, "zero division");
			return 1;
		}

		if (RIP < STACK_SIZE || RIP >= STACK_SIZE + CODE_SIZE)
		{
			fprintf(file_log, string_error, "memory access violation");
			return 1;
		}
		if (RF & e_exe_flag_exit)
			return 0;
		if (RF & e_exe_flag_error)
		{
			fprintf(file_log, string_error, "flag error was enabled");
			return 1;
		}
		LOG_D
	}
}

void executor_dump(FILE *stream, Executor *exe, int dump_stack, int dump_memory)
{
	fprintf(
		stream,
		"\nDUMP: Executor:");
	print_raw_data(
		stream, (char *)exe->memory + exe->r[e_exe_reg_rip] - 8, 16, 8);
	fprintf(
		stream,
		"\n\tmemory ptr:\t%p\n"
		"\t\trip: 0x%lx\n"
		"\t\trbp: 0x%lx\n"
		"\t\trsp: 0x%lx\n"
		"\t\trf:  0x%lx\n",
		exe->memory,
		exe->r[e_exe_reg_rip],
		exe->r[e_exe_reg_rbp],
		exe->r[e_exe_reg_rsp],
		exe->r[e_exe_reg_rf]);
	fprintf(stream, "\tshift reg: \t%lu\n", exe->shift_reg);
	for (size_t i = 0; i < 16; ++i)
		fprintf(stream, "\t\tr%02lu: 0x%lx\n", i, exe->r[i]);
	if (exe->r[e_exe_reg_rf] == 15) // double
		for (size_t i = 0; i < 16; ++i)
			fprintf(stream, "\t\tr%02lu: %lf\n", i, *(double *)&exe->r[i]);
	if (dump_stack)
		print_raw_data(stream, exe->memory, STACK_SIZE, 16);
	if (dump_memory)
		print_raw_data(stream, exe->memory, dump_memory, 16);
}