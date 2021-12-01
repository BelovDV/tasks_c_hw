#include "language.h"

#ifndef HEADER_CODER
#define HEADER_CODER

/**
 * @brief code line to binary dest
 * @return size of written data
 */
size_t coder_code(void *dest, Line *line);

/**
 * @brief decode coded by 'code' binary source to result
 * @return size of read data
 */
size_t coder_decode(void *source, Line *result);

#endif