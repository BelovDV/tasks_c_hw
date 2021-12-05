#include "utility.h"

#ifndef HEADER_EXECUTOR
#define HEADER_EXECUTOR

/**
 * @brief execute .dull binary file
 *
 * @param code binary file
 * @param log stderr for executor
 * @return (bool) was error
 */
int execute(Array_static_frame *code, FILE *log);

/**
 * @brief write info about executor to stream
 * @param stream there to write dump
 * @param executor Executor*
 * @param big will dump big (otherwise short)
 */
void executor_dump(FILE *stream, void *executor, int big);

#define STACK_SIZE 0x1000
#define STACK_OFFSET 0
#define CODE_SIZE 0x1000
#define CODE_OFFSET (STACK_SIZE + STACK_OFFSET)
#define MEMORY_SIZE 0x10000

#endif