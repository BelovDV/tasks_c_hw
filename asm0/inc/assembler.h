#include "text.h"
#include "dull.h"

#ifndef HEADER_ASSEMBLER
#define HEADER_ASSEMBLER

/**
 * @brief assemble program
 * 
 * @param text of program
 * @param dest - there to write result
 * @param log - there to write log
 * @return was error
 */
int assemble(Text_string_static text, struct Dull **dest,
			 FILE *log, size_t *size);

#endif