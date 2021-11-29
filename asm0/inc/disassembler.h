#include "text.h"
#include "utility.h"

#ifndef HEADER_DISASSEMBLER
#define HEADER_DISASSEMBLER

/**
 * @brief assemble program
 * 
 * @param bin binary .dull program
 * @param dest there to write disasm of binary
 * @param log there to write log
 * @return was error
 */
int disassemble(Array_static_frame bin, FILE *dest, FILE *log);

#endif