#include "text.h"
#include "utility.h"

#ifndef HEADER_ASSEMBLER
#define HEADER_ASSEMBLER

/**
 * @brief assemble program
 *
 * @param text of program
 * @param bin there to write result
 * @param log there to write log
 * @return was error
 */
int assemble(const Text_string *text, Array_static_frame *bin, FILE *log);

#endif
