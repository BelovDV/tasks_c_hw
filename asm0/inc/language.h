

#ifndef HEADER_LANGUAGE
#define HEADER_LANGUAGE

enum Instruction_id
{
	e_instr_id_error = -1,
	e_instr_id_nothing,
	e_instr_id_libcall,
	e_instr_id_syscall,
	e_instr_id_mov,

	e_instr_id_count,
};

enum
{
	e_const_language_max_word_length = 16,
};

extern const char instruction_names[][e_const_language_max_word_length];

enum Instruction_libcall_id
{
	e_libcall_id_write_int = 0,

};

extern const char libcall_names[][e_const_language_max_word_length];

enum Instruction_syscall_id
{
	e_syscall_id_exit = 0,

};

extern const char syscall_names[][e_const_language_max_word_length];

#endif