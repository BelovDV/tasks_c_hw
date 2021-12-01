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

#endif