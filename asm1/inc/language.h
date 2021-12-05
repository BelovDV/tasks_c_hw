#include <stdlib.h>
#include <stdio.h>

#ifndef HEADER_LANGUAGE
#define HEADER_LANGUAGE

// ===== // CONSTANTS // ===== //

enum
{
	e_max_word_length = 20,
	e_max_arg_count = 3,
	e_max_line_length = 60,
};

typedef size_t Word;

// ===== // KEYWORDS // ===== //

#define LANG_DECLARATIONS(FUNC) \
	FUNC(error)                 \
	FUNC(nope)                  \
	FUNC(libcall)               \
	FUNC(syscall)               \
	FUNC(mov)                   \
	FUNC(dump)                  \
	FUNC(mode)                  \
	FUNC(sub)                   \
	FUNC(div)                   \
	FUNC(add)                   \
	FUNC(mul)                   \
	FUNC(jmp)                   \
	FUNC(call)                  \
	FUNC(ret)                   \
	FUNC(if)                    \
	FUNC(push)                  \
	FUNC(pop)                   \
	FUNC(ifz)                   \
	FUNC(ifl)

#define LANG_INSTR_ENUM(name) e_lang_instr_##name,
enum
{
	LANG_DECLARATIONS(LANG_INSTR_ENUM)

		e_lang_instr_count
};
#undef LANG_INSTR_ENUM
#if e_lang_instr_count > 255
#error too many instructions
#endif

enum
{
	e_lang_key_error,

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
#if e_lang_key_count > 255
#error too many instructions
#endif

enum
{
	e_lang_mode_uint8,
	e_lang_mode_uint16,
	e_lang_mode_uint32,
	e_lang_mode_uint64,
	e_lang_mode_sint8,
	e_lang_mode_sint16,
	e_lang_mode_sint32,
	e_lang_mode_sint64,
	e_lang_mode_float,
	e_lang_mode_double,
};

enum
{
	e_section_none,
	e_section_text,
	e_section_data,
	e_section_error,
};

// ===== // INSTRUCTION // ===== //

enum
{
	// [rx+k*ry+c]
	e_arg_is_memory = 0x01,
	e_arg_is_ry = 0x02,
	e_arg_is_const_1 = 0x04,
	e_arg_is_const_2 = 0x08,

	e_arg_is__mask = 0x0F
};

typedef struct
{
	char mem;  // is memory
	char rx;   // e_exe_reg_...
	char k;	   // (0) 1 2 4 8
	char ry;   // e_exe_reg_...
	char is_c; // is c
	Word c;	   // uint64_t
} Argument;

typedef struct
{
	unsigned char id;
	unsigned char argc;
	Argument argv[e_max_arg_count];
} Instruction;

// ===== // LINE // ===== //

enum
{
	e_line_none,
	e_line_error,
	e_line_instr,
	e_line_data,
	e_line_label,
	e_line_section,
};

typedef struct
{
	int type; // e_line_...
	union
	{
		struct
		{
			size_t size;
			char data[e_max_line_length];
		} data;			   // data
		Instruction instr; // instr
		size_t offset;	   // label
	};
} Line;

// ===== // EXECUTOR // ===== //

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

	e_exe_reg_count, // reg none
};

enum
{
	e_exe_rf_mask_mode = 0xf, // e_lang_mode_...
	e_exe_rf_skip = 0x10,	  // after fallen if
	e_exe_rf_end = 0x20,	  // correct completion of program
	e_exe_rf_error = 0x40,	  // incorrect completion of program
};

/*
enum
{
	e_exe_error_nothing,				 // nothing error
	e_exe_error_instruction,			 // inst 'error' - set by language
	e_exe_error_execute_wrong_memory,	 // set by executor
	e_exe_error_memory_access_violation, // set by executor
	e_exe_error_unsupported,			 // set by language
	e_exe_error_wrong_arg_count,		 // set by language
	e_exe_error_wrong_arg_type,			 // set by language
	e_exe_error_wrong_keyword,			 // set by language
	e_exe_error_wrong_word_type,		 // set by language
	e_exe_error_zero_division,			 // set by language

	e_exe_error_mask = 0xff,

	e_exe_flag_if_failed = 0x0100, // skip next
	e_exe_flag_exit = 0x0200,	   // program complitted
	e_exe_flag_terminate = 0x0400, // program terminated - now, error occurred
};
*/

typedef struct
{
	// rip changed before instruction execution
	void *memory; // fixed size, fixed division
	Word shift;
	Word r[e_exe_reg_count];

	Line instr;
	Word constant[e_max_arg_count]; // if arg constant, it is here
	Word *v_ptr[e_max_arg_count];	// pointer to arg

	FILE *log;
} Executor;

// ===== // DESCRIPTION // ===== //

extern char mnemonics[e_lang_instr_count][e_max_word_length];
extern char language_keywords[e_lang_key_count][e_max_word_length];

int language_execute(Executor *exe);

#endif