#include "text.h"

#ifndef HEADER_DECODER
#define HEADER_DECODER

enum E_tokenization_condition
{
	e_token_section_text,
	e_token_section_data,
	e_token_section_nothing,

	e_token_flag_mask_intstr_changed = 0x01,
	e_token_flag_mask_data_changed = 0x02,
	e_token_flag_mask_end = 0x04,
	e_token_flag_mask_error = 0x08,
	e_token_flag_mask_start = 0x10,

};

enum E_argument
{
	e_argument_mask_is_memory = 0x01,
	e_argument_mask_is_rx = 0x02,
	e_argument_mask_is_ry = 0x04,
	e_argument_mask_is_const = 0x08
};

struct Argument
{
	// rx+k*ry+c	[rx+k*ry+c]
	uint8_t is_memory;
	uint8_t rx;
	uint8_t ry;
	uint8_t k;
	uint64_t constant; // if required
};

struct Instruction
{
	int id;
	int argc;
	struct Argument argv[3];
};

struct Tokenization_condition
{
	int error_id;
	int section;
	int flags;
	void *dictionary;
	union
	{
		struct Instruction last_instr;
		uint64_t data;
	};
};

enum
{
	e_const_tokenization_max_line_length = 79,
};

enum
{
	e_tokenization_error_nothing,
	e_tokenization_error_wrong_line,
	e_tokenization_error_unsupported,
	e_tokenization_error_deprecated,
	e_tokenization_error_wrong_section,
	e_tokenization_error_too_long_line,
	e_tokenization_error_multiply_start,

	e_tokenization_error_count
};

const extern char token_errors[e_tokenization_error_count][64];

/**
 * @brief initialise fields of struct
 * 
 * @param begin - pointer to the beginning of text
 * @param end - pointer to the end of text
 */
void tokenization_initialise(struct Tokenization_condition *condition);

/**
 * @brief read line and change condition
 * 
 * @param condition of current position
 * @param begin - position of line beginning
 * @param end - position of nearest new line symbol
 * 
 * @exception should be new line at the end?
 */
void tokenization_decode(struct Tokenization_condition *condition,
						 const char *begin, const char *end, size_t line);

#endif