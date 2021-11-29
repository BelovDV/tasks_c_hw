#include "utility.h"
// require executor.h instruction.h

#ifndef HEADER_LANGUAGE
#define HEADER_LANGUAGE

#define ASM0_VERSION 0

enum
{
	e_lang_max_word_len = 12,
	e_lang_max_line_length = 40,
};

enum
{
	e_lang_instr_error,

	e_lang_instr_nope,
	e_lang_instr_libcall,
	e_lang_instr_syscall,
	e_lang_instr_mov,
	e_lang_instr_dump,
	e_lang_instr_mode,
	e_lang_instr_sub,
	e_lang_instr_div,
	e_lang_instr_add,
	e_lang_instr_mul,
	e_lang_instr_jmp,
	e_lang_instr_call,
	e_lang_instr_ret,

	e_lang_instr_count // should be <= 255
};

enum
{
	e_lang_key_error = -1,

	e_lang_key_exit,
	e_lang_key_write_num,
	e_lang_key_read_num,
	e_lang_key_write_char,
	e_lang_key_read_char,
	e_lang_key_write_str,
	e_lang_key_read_str,
	e_lang_key_sqrt,

	e_lang_key_count
};

typedef struct
{
	char mnemonic[e_lang_max_word_len];
	/**
	 * @brief execute instr in condition executor
	 * @return was error
	 * @exception require executor.h instruction.h
	 */
	int (*exe)(void *executor, void *instr);
} Instruction_description;

extern Instruction_description language_instructions[e_lang_instr_count];
extern char language_keywords[e_lang_key_count][e_lang_max_word_len];

#endif