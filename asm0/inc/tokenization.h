#include "instruction.h"

#ifndef HEADER_DECODER
#define HEADER_DECODER

enum
{
	e_tok_mask_start_found = 0x01,
	e_tok_mask_changed_instr = 0x02,
	e_tok_mask_changed_data = 0x04,
	e_tok_mask_changed_label = 0x08,
	e_tok_mask_ready_jmp = 0x10,
};

typedef struct
{
	int error_id;
	int flags;
	Array_frame dictionary;
	union
	{
		Instruction last_instr;
		Word data;
	};
} Tokenization_condition;

enum
{
	e_tok_error_nothing,
	e_tok_error_unsupported,
	e_tok_error_deprecated,
	e_tok_error_too_long_line,
	e_tok_error_multiply_start,
	e_tok_error_wrong_line_start,
	e_tok_error_wrong_instr,
	e_tok_error_wrong_arg,
	e_tok_error_wrong_label,

	e_tok_error_count
};

extern const char token_errors[e_tok_error_count][64];

/**
 * @brief initialise fields of struct
 */
void tokenization_initialise(Tokenization_condition *condition);

/**
 * @brief read line and change condition
 * 
 * @param condition of current position
 * @param begin - position of line beginning
 * @param end - position of nearest new line symbol
 * @param second - (bool) is second pass (labels)
 * 
 * @exception end - new line, comment or eof
 */
void tokenization_decode(Tokenization_condition *condition,
						 const char *begin, const char *end,
						 int second);

typedef struct
{
	char name[e_lang_max_word_len]; // +1 (\0) -1(:)
	size_t position;
} Label;

#endif