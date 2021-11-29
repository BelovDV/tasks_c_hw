#include "executor.h"
#include "language.h"

#ifndef HEADER_INSTRUCTION
#define HEADER_INSTRUCTION

enum E_argument
{
	// [rx+k*ry+c]
	e_arg_is_memory = 0x01,
	e_arg_is_rx = 0x02,
	e_arg_is_ry = 0x04,
	e_arg_is_const = 0x08,

	e_arg_is__mask = 0x0F
};

enum
{
	e_const_max_arg_count = 3
};

typedef struct
{
	enum E_argument content; // [rx+k*ry+c]
	char rx;
	char ry;
	char k;
	Word c; // use if required
} Argument;

typedef struct
{
	int id;
	int argc;
	Argument argv[e_const_max_arg_count];
} Instruction;

/**
 * @brief translate instruction to bin
 * 
 * @return was error
 */
int instr_code(Instruction *instruction, Array_frame *dest);

/**
 * @brief translate instruction from bin
 * 
 * @return size occupied by instruction ; 0 if error
 */
size_t instr_decode(void *src, Instruction *result);

void instr_arg_dump(FILE *stream, Argument *arg, const char *prefix);
void instr_dump(FILE *stream, Instruction *instr, const char *prefix);

#endif