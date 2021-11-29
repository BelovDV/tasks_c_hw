#include "utility.h"

#ifndef HEADER_EXECUTOR
#define HEADER_EXECUTOR

#define R_COUNT 16		  // there are 16 general purpose registers
#define STACK_SIZE 0x1000 // located at memory[0]
#define CODE_SIZE 0x3000
#define MEMORY_SIZE 0x1000000
#define NUMBER_ITERATIONS 1000 // Debug

enum Registers
{
	e_exe_reg_r0,
	e_exe_reg_r1,
	e_exe_reg_r2,
	e_exe_reg_r3,
	e_exe_reg_r4,
	e_exe_reg_r5,
	e_exe_reg_r6,
	e_exe_reg_r7,

	e_exe_reg_r8,
	e_exe_reg_r9,
	e_exe_reg_r10,
	e_exe_reg_r11,
	e_exe_reg_r12,
	e_exe_reg_r13,
	e_exe_reg_r14,
	e_exe_reg_r15,

	e_exe_reg_rip,
	e_exe_reg_rbp,
	e_exe_reg_rsp,
	e_exe_reg_rf,

	e_exe_reg_count,
};

typedef unsigned long Word;

enum
{
	e_exe_flags_word = 0x0F,
	e_exe_flag_word_size_1 = 0x01,
	e_exe_flag_word_size_2 = 0x02,
	e_exe_flag_word_signed = 0x04,
	e_exe_flag_word_float = 0x08,
	e_exe_flag_exit = 0x10,
	e_exe_flag_error = 0x20,
};

enum
{
	e_exe_error_memory_access_violation = 0x01,
	e_exe_error_wrong_word_type = 0x02,
	e_exe_error_unsupported = 0x04,
	e_exe_error_wrong_keyword = 0x08,
	e_exe_error_wrong_argc = 0x10,
	e_exe_error_wrong_arg_type = 0x20,
	e_exe_error_wrong_const = 0x40,
	e_exe_error_zero_division = 0x80,
};

typedef struct
{
	void *memory;
	Word error;

	size_t shift_reg;
	Word r[e_exe_reg_count];
} Executor;

/**
 * @brief execute .dull binary file
 * 
 * @param code binary file
 * @param log stderr for executor
 * @return (bool) was error
 */
int execute(Array_static_frame *code, FILE *log);

/**
 * @brief store executor's information
 */
void executor_dump(FILE *out, Executor *exe, int dump_stack, int dump_memory);

#endif